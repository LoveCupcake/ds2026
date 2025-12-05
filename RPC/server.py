from xmlrpc.server import SimpleXMLRPCServer
import base64

def upload_file(filename, data):
    file_bytes = base64.b64decode(data.data)

    with open(filename, "wb") as f:
        f.write(file_bytes)

    print(f"File received: {filename}")
    return "OK"

server = SimpleXMLRPCServer(("0.0.0.0", 9000))
server.register_function(upload_file, "upload_file")

print("RPC Server listening on port 9000...")
server.serve_forever()
