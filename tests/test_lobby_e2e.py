import asyncio
import json
import os

import pytest
from websockets import connect

from server import lobby_server as server

# mark tests as asyncio
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


async def test_e2e_flow_with_mid_join_request():
    """End-to-end simulated flow:
    - Two players join -> receive PROMPT_START -> both confirm -> SESSION_START
    - Host chooses game -> both receive GAME_LAUNCH
    - A third player joins mid-session -> existing players receive JOIN_REQUEST
    """
    stop_event, task = await start_test_server(timeout_seconds=3)
    try:
        # initial two players
        p1 = await connect_and_join("Alpha")
        p2 = await connect_and_join("Beta")

        # prompt should arrive
        msg1 = await wait_for_action(p1, "PROMPT_START", timeout=1.0)
        msg2 = await wait_for_action(p2, "PROMPT_START", timeout=1.0)
        assert msg1.get("reason") == "enough_players"

        # both confirm
        await p1.send(json.dumps({"action": "CONFIRM_START"}))
        await p2.send(json.dumps({"action": "CONFIRM_START"}))

        # session starts
        s1 = await wait_for_action(p1, "SESSION_START", timeout=1.0)
        s2 = await wait_for_action(p2, "SESSION_START", timeout=1.0)
        assert s1.get("playerCount") == 2
        assert s2.get("playerCount") == 2
        host_name = s1.get("host")
        assert host_name in ("Alpha", "Beta")

        # non-host attempts to choose -> receives ERROR
        non_host = p2 if host_name == "Alpha" else p1
        await non_host.send(json.dumps({"action": "GAME_CHOSEN", "gameId": "not_allowed"}))
        err = await wait_for_action(non_host, "ERROR", timeout=1.0)
        assert err.get("code") == "NOT_HOST"

        # host chooses a game -> everyone gets GAME_LAUNCH
        host_ws = p1 if host_name == "Alpha" else p2
        await host_ws.send(json.dumps({"action": "GAME_CHOSEN", "gameId": "the-game"}))
        gl1 = await wait_for_action(p1, "GAME_LAUNCH", timeout=1.0)
        gl2 = await wait_for_action(p2, "GAME_LAUNCH", timeout=1.0)
        assert gl1.get("gameId") == "the-game"
        assert gl2.get("gameId") == "the-game"

        # mid-session join request
        p3 = await connect_and_join("Gamma")
        # existing players should receive JOIN_REQUEST
        jr1 = await wait_for_action(p1, "JOIN_REQUEST", timeout=1.0)
        jr2 = await wait_for_action(p2, "JOIN_REQUEST", timeout=1.0)
        assert jr1.get("name") == "Gamma"
        assert jr2.get("name") == "Gamma"

        await p1.close()
        await p2.close()
        await p3.close()
    finally:
        await shutdown_test_server(stop_event, task)
