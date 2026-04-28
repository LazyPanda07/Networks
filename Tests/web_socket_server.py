from simple_websocket_server import WebSocketServer, WebSocket


class Echo(WebSocket):
    def handle(self):
        self.send_message(self.data)


server = WebSocketServer("127.0.0.1", 5050, Echo)
server.serve_forever()
