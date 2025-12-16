import asyncio
import json
import os

import pytest
from websockets import connect

from server import lobby_server as server

# mark all tests as asyncio
pytestmark = pytest.mark.asyncio

SERVER_HOST = os.getenv("LOBBY_TEST_HOST", "127.0.0.1")
SERVER_PORT = int(os.getenv("LOBBY_TEST_PORT", "12345"))
WS_URL = f"ws://{SERVER_HOST}:{SERVER_PORT}"


async def wait_for_action(ws, action, timeout=2.0):
    end = asyncio.get_event_loop().time() + timeout
    while True:
        if asyncio.get_event_loop().time() > end:
            raise asyncio.TimeoutError(f"Timed out waiting for action {action}")
        raw = await asyncio.wait_for(ws.recv(), timeout=end - asyncio.get_event_loop().time())
        try:
            msg = json.loads(raw)
        except Exception:
            continue
        if msg.get("action") == action:
            return msg


async def start_test_server(timeout_seconds=1):
    stop_event = asyncio.Event()
    task = asyncio.create_task(server.run(host=SERVER_HOST, port=SERVER_PORT, confirm_timeout=timeout_seconds, stop_event=stop_event))
    # wait briefly for server to be ready
    await asyncio.sleep(0.05)
    return stop_event, task


async def shutdown_test_server(stop_event, task):
    stop_event.set()
    try:
        await asyncio.wait_for(task, timeout=1.0)
    except asyncio.TimeoutError:
        task.cancel()


async def connect_and_join(name):
    ws = await connect(WS_URL)
    await ws.send(json.dumps({"action": "JOIN", "name": name}))
    return ws


async def drain_initial_updates(ws, expected_min=1, timeout=0.5):
    # consume any initial LOBBY_UPDATEs; we don't assert contents here
    start = asyncio.get_event_loop().time()
    while asyncio.get_event_loop().time() - start < timeout and expected_min > 0:
        try:
            raw = await asyncio.wait_for(ws.recv(), timeout=0.05)
            expected_min -= 1
        except asyncio.TimeoutError:
            break


async def test_simple_join_and_confirm():
    stop_event, task = await start_test_server(timeout_seconds=3)
    try:
        a = await connect_and_join("Dad")
        b = await connect_and_join("Kid")
        # each client should eventually receive PROMPT_START
        msg_a = await wait_for_action(a, "PROMPT_START", timeout=1.0)
        assert msg_a.get("reason") == "enough_players"
        msg_b = await wait_for_action(b, "PROMPT_START", timeout=1.0)
        # both confirm
        await a.send(json.dumps({"action": "CONFIRM_START"}))
        await b.send(json.dumps({"action": "CONFIRM_START"}))
        # both should receive SESSION_START
        s_a = await wait_for_action(a, "SESSION_START", timeout=1.0)
        s_b = await wait_for_action(b, "SESSION_START", timeout=1.0)
        assert s_a.get("playerCount") == 2
        assert s_b.get("playerCount") == 2
        await a.close()
        await b.close()
    finally:
        await shutdown_test_server(stop_event, task)


async def test_decline_removes_player():
    stop_event, task = await start_test_server(timeout_seconds=3)
    try:
        a = await connect_and_join("Dad")
        b = await connect_and_join("Kid")
        await wait_for_action(a, "PROMPT_START", timeout=1.0)
        await wait_for_action(b, "PROMPT_START", timeout=1.0)
        # Kid declines
        await b.send(json.dumps({"action": "DECLINE_START"}))
        # Dad should see PLAYER_LEFT
        pl = await wait_for_action(a, "PLAYER_LEFT", timeout=1.0)
        assert pl.get("leftName") == "Kid"
        # No SESSION_START should be sent
        with pytest.raises(asyncio.TimeoutError):
            await wait_for_action(a, "SESSION_START", timeout=0.5)
        await a.close()
        await b.close()
    finally:
        await shutdown_test_server(stop_event, task)


async def test_timeout_removes_unresponsive():
    # confirmation timeout set to 1 second in server instance
    stop_event, task = await start_test_server(timeout_seconds=1)
    try:
        a = await connect_and_join("Dad")
        b = await connect_and_join("Kid")
        await wait_for_action(a, "PROMPT_START", timeout=1.0)
        # Dad confirms, Kid does not
        await a.send(json.dumps({"action": "CONFIRM_START"}))
        # After timeout, Kid should be removed (Dad sees PLAYER_LEFT)
        pl = await wait_for_action(a, "PLAYER_LEFT", timeout=2.0)
        assert pl.get("leftName") == "Kid"
        # No session start
        with pytest.raises(asyncio.TimeoutError):
            await wait_for_action(a, "SESSION_START", timeout=0.5)
        await a.close()
        await b.close()
    finally:
        await shutdown_test_server(stop_event, task)


async def test_host_and_game_choice():
    stop_event, task = await start_test_server(timeout_seconds=3)
    try:
        a = await connect_and_join("Dad")
        b = await connect_and_join("Kid")
        await wait_for_action(a, "PROMPT_START", timeout=1.0)
        await a.send(json.dumps({"action": "CONFIRM_START"}))
        await b.send(json.dumps({"action": "CONFIRM_START"}))
        s = await wait_for_action(a, "SESSION_START", timeout=1.0)
        host_name = s.get("host")
        assert host_name in ("Dad", "Kid")
        # If non-host tries to choose a game, they get an ERROR
        non_host_ws = b if host_name == "Dad" else a
        await non_host_ws.send(json.dumps({"action": "GAME_CHOSEN", "gameId": "foo"}))
        err = await wait_for_action(non_host_ws, "ERROR", timeout=1.0)
        assert err.get("code") == "NOT_HOST"
        # Host chooses a game -> everyone receives GAME_LAUNCH
        host_ws = a if host_name == "Dad" else b
        await host_ws.send(json.dumps({"action": "GAME_CHOSEN", "gameId": "supergame"}))
        gl_a = await wait_for_action(a, "GAME_LAUNCH", timeout=1.0)
        gl_b = await wait_for_action(b, "GAME_LAUNCH", timeout=1.0)
        assert gl_a.get("gameId") == "supergame"
        assert gl_b.get("gameId") == "supergame"
        await a.close()
        await b.close()
    finally:
        await shutdown_test_server(stop_event, task)
