import socket
import struct
import time
import random
import site
import crc32c

HEAD_SIZE = 18
META_SIZE = HEAD_SIZE + 4 # Includes CRC

'''
Terminology clarificiation:
	bigpak: collection of multiple packets that encode msg
	packet: UDP packet, consisting of a header, blk, and crc32c
	header: at head of each packet, contains data about bigpak and packet
		- password
		- total_len: total length of msg being sent
		- blk_len: blk length
		- bigpak_ind: index of bigpak (helps keep different bigpaks separate)
		- blk_ind: index of blk within packet
		- connhash: number that helps differentiate between different connections
	blk: data within UDP packet
'''

class BigPak():

	error = socket.error

	def __init__(self, host, send_port = 8080, recv_port = 8080, error_checking = True,
	      blocking = False, size = 8970, hz = 60000000):
		self.ec = error_checking
		self.size = size # <= 9126 on Mac by default
		self.hz = hz

		self.last_pak_time = 0
		self.bigpak_ind = 1
		self.connhash = random.randint(0, 65535)
		self.conn_id = -1
		self.last_recv_pak_time = 0
		self.blocking = blocking

		self.sk = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.sk.setblocking(True)
		self.sk.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

		self.sk.bind(('', recv_port))

		self.connection = (host, send_port)

	def send_msg(self, msg, debug = False):
		self.sk.setblocking(True)
		time_now = time.process_time_ns()/1000000 #milliseconds
		blk_len = max(self.size-META_SIZE,64)
		total_len = len(msg)
		blk_ind = 0
		ec_packet = None # error checking packet
		send_ec = False
		
		while (time_now - self.last_pak_time > 1):
			self.last_pak_time += 1/self.hz

			packet_data_len = min(total_len-blk_len*blk_ind, blk_len)

			# Create header
			endian = 'little'
			password = 0x24a4894b #'K��$' ('KenS')

			packet = password.to_bytes(4, endian)
			packet += len(msg).to_bytes(4, endian)
			packet += blk_len.to_bytes(2, endian)
			packet += self.bigpak_ind.to_bytes(2, endian)
			packet += blk_ind.to_bytes(4, endian)
			packet += self.connhash.to_bytes(2, endian)

			# Add data
			if send_ec:
				packet += ec_packet.to_bytes(blk_len, endian)
			else:
				data = msg[blk_len*blk_ind:blk_len*blk_ind + packet_data_len]
				packet += data
				if self.ec:
					if blk_ind == 0:
						ec_packet = int.from_bytes(data, endian)
					else:
						ec_packet ^= int.from_bytes(data, endian)

			# Add CRC32C
			crc = crc32c.crc32c(packet) ^ 0xFFFFFFFF
			packet += crc.to_bytes(4, endian)

			if not debug:
				self.sk.sendto(packet, self.connection)

			blk_ind += 1
			if blk_len*blk_ind  >= total_len:
				if self.ec and not send_ec:
					send_ec = True
				else:
					break

		self.bigpak_ind += 1
		if self.bigpak_ind > 0xFFFF:
			self.bigpak_ind = 0
 
	def recv_msg(self):
		cur_bigpak = -1
		blks = [] # data of received blks
		blks_recv = [] # Boolean array of what blks have been received
		nblk_recv = 0 # number of blks received
		self.sk.setblocking(self.blocking)

		while True:
			packet, addr = self.sk.recvfrom(65536)
			if addr == None:
				return 0
			
			# Get header info
			recv_len = len(packet)
			total_len = struct.unpack('<I', packet[4:8])[0]
			blk_len = struct.unpack('<H', packet[8:10])[0]
			bigpak_ind = struct.unpack('<H', packet[10:12])[0]
			blk_ind = struct.unpack('<I', packet[12:16])[0]
			cur_conn_id = struct.unpack('<H', packet[16:18])[0]

			if blk_len <= 0:
				continue # error!

			nblk = int((total_len+(blk_len-1))/blk_len) # number of blks (= ceil(total_len/blk_len))

			if blk_ind < nblk:
				packet_data_len = min(total_len-blk_len*blk_ind,blk_len)
			else:
				packet_data_len = blk_len # error checking packet
				if cur_bigpak == -1:
					continue # this means we got error checking packet from previous bigpak
		
			crc = struct.unpack('I', packet[packet_data_len+HEAD_SIZE:packet_data_len+HEAD_SIZE+4])[0]
			if blk_ind > nblk:
				continue
			if (total_len <= 0) or (blk_len >= 65536-META_SIZE) or (packet_data_len != recv_len-META_SIZE):
				continue
			if crc32c.crc32c(packet[0:packet_data_len+HEAD_SIZE]) ^ 0xFFFFFFFF != crc:
				continue
			
			# if it has been sometime since receiving, reset everything
			time_now = time.process_time_ns()/1000000
			if abs(time_now-self.last_recv_pak_time) >= 3000:
				self.conn_id = cur_conn_id
				cur_bigpak = -1
			self.last_recv_pak_time = time_now

			if self.conn_id == -1:
				self.conn_id = cur_conn_id # first connection!
			elif cur_conn_id != self.conn_id:
				continue # throw out packet from other connection

			if cur_bigpak == -1: # new bigpak
				cur_bigpak = bigpak_ind
				blks = []
				blks_recv = []
				nblk_recv = 0
				for i in range(nblk + 1): # +1 for error checking packet
					blks.append(b'')
					blks_recv.append(False)
			elif cur_bigpak != bigpak_ind:
				continue # this should hopefully not happen
			
			blks[blk_ind] = packet[HEAD_SIZE:HEAD_SIZE+packet_data_len]
			
			if packet_data_len < blk_len:
				blks[blk_ind] += bytearray(blk_len - packet_data_len) # pad with zeros

			if not blks_recv[blk_ind]:
				blks_recv[blk_ind] = True
				nblk_recv += 1
			
			if nblk_recv == nblk:
				# either got all blks perfectly or one wrong and error checking
				for bad_blk_ind in range(nblk):
					if not blks_recv[bad_blk_ind]:
						blks[bad_blk_ind] = bytearray(blk_len)
						for j in range(nblk + 1):
							if j != bad_blk_ind:
								for k in range(blk_len):
									blks[bad_blk_ind][k] ^= blks[j][k]


				cur_bigpak = -1
				return b''.join(i for i in blks[0:nblk])[0:total_len]
		
	def close(self):
		self.sk.close()
	
	def reopen(self):
		self.sk = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.sk.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
		self.sk.setblocking(self.blocking)
		self.sk.connect((self.host, self.port))
