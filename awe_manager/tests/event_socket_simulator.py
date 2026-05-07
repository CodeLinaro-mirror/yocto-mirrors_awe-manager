import logging
import socket
import threading
import time


logger = logging.getLogger(__file__)


class EventSocketSimulator(threading.Thread):
    def __init__(self, single_shot, name="tst", port=15011, data=b''):
        threading.Thread.__init__(self)
        self.daemon = True
        self.single_shot = single_shot
        self.name = name
        self.host = '127.0.0.1'
        self.port = port
        self.data = data
        self._stop_flag = threading.Event()

    def run(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.settimeout(1.0)
        server_socket.bind((self.host, self.port))
        server_socket.listen(1)
        while not self._stop_flag.is_set():
            logger.info(f"{self.name}:: Listening on {self.host}:{self.port}...")
            try:
                client_socket, client_address = server_socket.accept()
            except socket.timeout:
                continue
            logger.info(f"{self.name}:: Connection from {client_address}")
            logger.info(f"{self.name}:: Sending data: {self.data}")
            client_socket.sendall(self.data)
            time.sleep(1.0)
            client_socket.close()
            logger.info(f"{self.name}:: Closed")
            if self.single_shot:
                break
        server_socket.close()


class TestServers:
    def __init__(self, single_shot=False):
        # header with 0 datasize, rest of data is don't care
        hdr = bytearray(b'\xfe\xaf\xad\xde')  # magicword
        hdr += b'\x00\x00\x00\x00' # instanceId
        hdr += b'\x00\x00\x00\x00' # objectId;          // event module objectId;
        hdr += b'\x00\x00\x00\x00' # classId;           // event module classid, 0 or don't care for system events (eventCategory != 0), 
        hdr += b'\x00\x00\x00\x00' # eventType;         // event type as defined in event module
        hdr += b'\x00\x00\x00\x00' # eventCategory;     // 0 = user/event 1 = system
        hdr += b'\x00\x00\x00\x00' # dataSize;          // size of payload data in Bytes
        hdr += b'\x00\x00\x00\x00\x00\x00\x00\x00' # timeStamp;         // systick

        # header with payload size > default 264
        hdrb = bytearray(b'\xfe\xaf\xad\xde')  # magicword
        hdrb += b'\x00\x00\x00\x00' # instanceId
        hdrb += b'\x00\x00\x00\x00' # objectId;          // event module objectId;
        hdrb += b'\x00\x00\x00\x00' # classId;           // event module classid, 0 or don't care for system events (eventCategory != 0), 
        hdrb += b'\x00\x00\x00\x00' # eventType;         // event type as defined in event module
        hdrb += b'\x00\x00\x00\x00' # eventCategory;     // 0 = user/event 1 = system
        hdrb += b'\x00\x10\x00\x00' # dataSize;          // size of payload data in Bytes
        hdrb += b'\x00\x00\x00\x00\x00\x00\x00\x00' # timeStamp;         // systick

        self.use_cases = [
            EventSocketSimulator(single_shot, name="NoMagic", port=15011, data=b'\x01\x02\x03\x04'),
            EventSocketSimulator(single_shot, name="NoHeader", port=15012, data=b'\xfe\xaf\xad\xde\x00\x00\x00\x00'),
            EventSocketSimulator(single_shot, name="NoPayload", port=15013, data=bytes(hdr)),
            EventSocketSimulator(single_shot, name="PayloadResize", port=15014, data=bytes(hdrb))
        ]

    def start(self):
        [uc.start() for uc in self.use_cases]
        logger.info("All simulators started")

    def stop(self):
        for uc in self.use_cases:
            uc._stop_flag.set()
        for uc in self.use_cases:
            uc.join(5.0)
        logger.info("All simulators stopped")


if __name__ == "__main__":

    logging.basicConfig(level=logging.INFO, format='%(levelname)-7s # %(module)-10s :: %(message)s')

    ts = TestServers()
    ts.start()
    ts.stop()
