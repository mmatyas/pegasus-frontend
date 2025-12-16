"""Santop Lobby Server (asyncio + websockets)

Usage (dev):
    python server/lobby_server.py

The server exposes `run(host, port, confirm_timeout, stop_event)` which can be used
by tests to run the server in-process and stop it via the `stop_event`.
"""
import asyncio
import json
import logging
import os
import time
from typing import Dict, Any, Optional

from websockets import serve, ServerProtocol, ConnectionClosed

LOG = logging.getLogger("santop.lobby")
LOG.setLevel(logging.DEBUG)
handler = logging.StreamHandler()
handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)s %(message)s"))
LOG.addHandler(handler)

# Globals (kept simple for prototype)
players: Dict[ServerProtocol, Dict[str, Any]] = {}
players_lock = asyncio.Lock()
confirmations_pending = False
session_active = False
confirmation_task: Optional[asyncio.Task] = None
confirmation_timeout_seconds = int(os.getenv("LOBBY_CONFIRM_TIMEOUT", "600"))
host_ws: Optional[ServerProtocol] = None


def info_for_client(info: Dict[str, Any]) -> Dict[str, Any]:
    """Return a sanitized view of a player dict suitable for clients."""
    return {
        "name": info.get("name"),
        "id": info.get("id"),
        "status": info.get("status"),
    }


async def notify_all(message: dict):
    if not players:
        return
    msg_text = json.dumps(message)
    LOG.debug("Broadcasting: %s", msg_text)
    # Send to each client individually so that a failing send doesn't
    # cancel all other sends; collect disconnected sockets for cleanup.
    to_remove = []
    for ws in list(players.keys()):
        try:
            await ws.send(msg_text)
        except Exception as exc:
            LOG.debug("Failed to send to %s: %s", players.get(ws, {}).get("name"), exc)
            to_remove.append(ws)
    # Remove any sockets that errored during send
    for ws in to_remove:
        try:
            await remove_player(ws, reason="send_failed")
        except Exception:
            LOG.debug("Error while removing failed socket")


async def remove_player(ws: ServerProtocol, reason: str = "left"):
    global confirmations_pending, session_active
    async with players_lock:
        if ws in players:
            name = players[ws].get("name")
            players.pop(ws, None)
            LOG.info("Removed player %s (%s). Remaining: %d", name, reason, len(players))
            # Notify others
            await notify_all({
                "action": "PLAYER_LEFT",
                "leftName": name,
                "reason": reason,
                "players": [info_for_client(p) for p in players.values()]
            })
            if len(players) < 2:
                confirmations_pending = False
                session_active = False
                # cancel confirmation task if any
                await cancel_confirmation_task()
                # If the removed player was the host, clear host
                global host_ws
                if host_ws == ws:
                    host_ws = None

async def cancel_confirmation_task():
    global confirmation_task
    if confirmation_task and not confirmation_task.done():
        confirmation_task.cancel()
        try:
            await confirmation_task
        except asyncio.CancelledError:
            LOG.debug("Canceled confirmation task")
    confirmation_task = None


async def start_confirmation_timeout(timeout_seconds: int):
    global confirmation_task, confirmations_pending
    LOG.debug("Starting confirmation timeout: %ds", timeout_seconds)

    async def _timeout():
        global confirmations_pending
        try:
            await asyncio.sleep(timeout_seconds)
            if confirmations_pending:
                LOG.info("Confirmation timeout reached; removing unresponsive players")
                async with players_lock:
                    to_remove = [ws for ws, info in list(players.items()) if not info.get("confirmed")]
                for ws in to_remove:
                    await remove_player(ws, reason="timeout")
                confirmations_pending = False
        except asyncio.CancelledError:
            LOG.debug("Confirmation timeout cancelled")

    # Cancel previous
    await cancel_confirmation_task()
    confirmation_task = asyncio.create_task(_timeout())


async def handle_messages(ws: ServerProtocol):
    global confirmations_pending, session_active
    try:
        async for raw in ws:
            try:
                msg = json.loads(raw)
            except Exception:
                LOG.warning("Invalid JSON from %s: %s", ws.remote_address, raw)
                await ws.send(json.dumps({"action": "ERROR", "message": "invalid_json"}))
                continue
            action = msg.get("action")
            LOG.debug("Received %s from %s", action, players.get(ws, {}).get("name"))
            # update last_active
            async with players_lock:
                if ws in players:
                    players[ws]["last_active"] = int(time.time())
            if action == "CONFIRM_START":
                async with players_lock:
                    if ws in players:
                        players[ws]["confirmed"] = True
                # Check if all active players confirmed
                async with players_lock:
                    all_confirmed = all(info.get("confirmed") for info in players.values()) and len(players) >= 2
                if confirmations_pending and all_confirmed:
                    confirmations_pending = False
                    session_active = True
                    # clear confirmed flags
                    async with players_lock:
                        for info in players.values():
                            info.pop("confirmed", None)
                    # select host as first connected player (deterministic insertion order)
                    async with players_lock:
                        host_candidate = next(iter(players.values())) if players else None
                        host_name = host_candidate.get("name") if host_candidate else None
                    # set host_ws for authorization checks
                    global host_ws
                    async with players_lock:
                        # find ws by matching host name (first occurrence)
                        host_ws = None
                        for ws_it, info in players.items():
                            if info.get("name") == host_name:
                                host_ws = ws_it
                                break
                    await cancel_confirmation_task()
                    await notify_all({"action": "SESSION_START", "playerCount": len(players), "host": host_name})
            elif action == "DECLINE_START":
                # remove the decliner
                await remove_player(ws, reason="declined")
                confirmations_pending = False
                session_active = False
                await cancel_confirmation_task()
            elif action == "GAME_CHOSEN":
                game_id = msg.get("gameId")
                # only the host is allowed to finalize game choice
                if ws != host_ws:
                    LOG.info("Non-host attempted GAME_CHOSEN: %s", players.get(ws, {}).get("name"))
                    try:
                        await ws.send(json.dumps({"action": "ERROR", "code": "NOT_HOST", "message": "Only host may choose game"}))
                    except Exception:
                        pass
                else:
                    await notify_all({"action": "GAME_LAUNCH", "gameId": game_id})
            elif action == "LEAVE":
                await remove_player(ws, reason="leave")
            elif action == "HEARTBEAT":
                # Noop beyond updating last_active
                pass
            # else: ignore unknown actions for now
    except ConnectionClosed:
        LOG.debug("ConnectionClosed in handle_messages for %s", ws)
    except Exception as e:
        LOG.exception("Error in handle_messages: %s", e)
        try:
            await ws.send(json.dumps({"action": "ERROR", "message": "server_error"}))
        except Exception:
            pass


async def handle_client(ws: ServerProtocol, path: Optional[str] = None):
    global confirmations_pending, session_active
    # Expect first a JOIN (but if not, we accept default name)
    try:
        raw = await ws.recv()
    except ConnectionClosed:
        return
    try:
        msg = json.loads(raw)
    except Exception:
        msg = {}
    if msg.get("action") == "JOIN":
        name = msg.get("name", "Player")
        pid = msg.get("playerId")
    else:
        name = "Player"
        pid = None
    async with players_lock:
        players[ws] = {"id": pid, "name": name, "status": "waiting", "last_active": int(time.time())}
    LOG.info("Player joined: %s %s", name, ws.remote_address)
    # Broadcast update
    await notify_all({"action": "LOBBY_UPDATE", "players": [info_for_client(p) for p in players.values()]})

    # Decide whether to prompt
    async with players_lock:
        num = len(players)
    if not session_active and num >= 2:
        confirmations_pending = True
        await notify_all({"action": "PROMPT_START", "reason": "enough_players"})
        # start timeout
        await start_confirmation_timeout(confirmation_timeout_seconds)
    elif session_active:
        # join request during active session
        confirmations_pending = True
        await notify_all({"action": "JOIN_REQUEST", "name": name})
        await start_confirmation_timeout(confirmation_timeout_seconds)

    try:
        await handle_messages(ws)
    finally:
        # cleanup on disconnect
        await remove_player(ws, reason="disconnect")


async def run(host: str = "127.0.0.1", port: int = 12345, confirm_timeout: Optional[int] = None, stop_event: Optional[asyncio.Event] = None):
    global confirmation_timeout_seconds
    if confirm_timeout is not None:
        confirmation_timeout_seconds = confirm_timeout
    stop_event = stop_event or asyncio.Event()
    LOG.info("Starting Santop Lobby Server on %s:%d (confirm_timeout=%ds)", host, port, confirmation_timeout_seconds)
    async with serve(handle_client, host, port):
        await stop_event.wait()
    LOG.info("Santop Lobby Server stopped")


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=12345)
    parser.add_argument("--confirm-timeout", type=int, default=confirmation_timeout_seconds)
    args = parser.parse_args()

    stop = asyncio.Event()

    try:
        asyncio.run(run(host=args.host, port=args.port, confirm_timeout=args.confirm_timeout, stop_event=stop))
    except KeyboardInterrupt:
        LOG.info("Interrupted; shutting down")
