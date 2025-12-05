import sys
import base64
from xmlrpc.client import ServerProxy, Binary

if len(sys.argv) < 2:
    print("Usage: python3 client.py <file>")
    sys.exit(1)

filename = sys.argv[1]

with open(filename, "rb") as f:
    data = f.read()

encoded_data = Binary(base64.b64encode(data))

server = ServerProxy("http://127.0.0.1:9000/")
response = server.upload_file(filename, encoded_data)

print("Server Response:", response)
