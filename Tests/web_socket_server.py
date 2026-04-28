from simple_websocket_server import WebSocketServer, WebSocket


class Echo(WebSocket):
    def handle(self):
        self.send_message(self.data)


server = WebSocketServer("127.0.0.1", 5050, Echo)

open("web_socket_server_ready.txt", "w").close()

server.serve_forever()
