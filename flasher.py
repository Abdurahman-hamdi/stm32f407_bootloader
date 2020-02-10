from __future__ import print_function

import sys, serial, binascii, time, struct, thread, argparse

from intelhex import IntelHex






ACK       = 0x79
NACK      = 0x1F
CMD_ERASE = 0x43
CMD_GETID = 0x02
CMD_WRITE = 0x2b
CMD_jump  = 0x44
CMD_wr_p  = 0x33
RE_wr_pr  = 0x55
up_date   = 0x66



class ProgramModeError(Exception):
    pass

class TimeoutError(Exception):
    pass

class STM32Flasher(object):
    def __init__(self, serialPort, baudrate=38400):
        self.serial = serial.Serial(serialPort, baudrate=baudrate, timeout=30)
    
	
	
    def _sstr_(self,data):
       
       
		
        self.serial.write(data)
        ky = input("press any key to start\n")
      
    def _crc_stm32(self, data):
        #Computes CRC checksum using CRC-32 polynomial 
        crc = 0xFFFFFFFF

        for d in data:
            crc ^= d
            for i in range(32):
                if crc & 0x80000000:
                    crc = (crc << 1) ^ 0x04C11DB7 #Polynomial used in STM32
                else:
                    crc = (crc << 1)

        return (crc & 0xFFFFFFFF)

    def _create_cmd_message(self, msg):
        #Encodes a command adding the CRC32
        cmd = list(msg) + list(struct.pack("I", self._crc_stm32(msg)))
        return cmd
    
	
    def _write_protection(self, sector):
        self.serial.flushInput()
        self.serial.write(self._create_cmd_message((CMD_wr_p,sector)))
        data = self.serial.read(1)
        if len(data) == 1:
            if struct.unpack("b", data)[0] != ACK:
                raise ProgramModeError("Can't remove write protection")
        else:
            raise TimeoutError("Timeout error")
    def _Re_write_protection(self, sector):
        self.serial.flushInput()
        self.serial.write(self._create_cmd_message((RE_wr_pr,sector)))
        data = self.serial.read(1)
        if len(data) == 1:
            if struct.unpack("b", data)[0] != ACK:
                raise ProgramModeError("Can't remove write protection")
        else:
            raise TimeoutError("Timeout error")
	
    def _jump(self, address):
        self.serial.flushInput()
        self.serial.write(self._create_cmd_message(([CMD_jump] +map(ord, struct.pack("I", address)))))
        data = self.serial.read(1)
        if len(data) == 1:
            if struct.unpack("b", data)[0] != ACK:
                raise ProgramModeError("Can't jump")
        else:
            raise TimeoutError("Timeout error")
    def _update(self, address):
        self.serial.flushInput()
        self.serial.write(self._create_cmd_message(([up_date] +map(ord, struct.pack("I", address)))))
        data = self.serial.read(1)
        if len(data) == 1:
            if struct.unpack("b", data)[0] != ACK:
                raise ProgramModeError("Can't update")
        else:
            raise TimeoutError("Timeout error")

    def eraseFLASH(self, nsectors=0x20):
        #Sends an CMD_ERASE to the bootloader
        self.serial.flushInput()
        self.serial.write(self._create_cmd_message((CMD_ERASE,nsectors)))
        data = self.serial.read(1)
        if len(data) == 1:
            if struct.unpack("b", data)[0] != ACK:
                raise ProgramModeError("Can't erase FLASH")
        else:
            raise TimeoutError("Timeout error")
      
    

    def writeImage(self, filename):
        #Sends an CMD_WRITE to the bootloader
        #This is method is a generator, that returns its progresses to the caller.
        #In this way, it's possible for the caller to live-print messages about
        #writing progress 
        ih = IntelHex()  
        ih.loadhex(filename)
        yield {"saddr": ih.minaddr(), "eaddr": ih.maxaddr()}
        global sad
        addr = ih.minaddr()
        sad=addr
        content = ih.todict()
        abort = False
        resend = 0
        while addr <= ih.maxaddr():
            if not resend:
                data = []
                saddr = addr
                for i in range(16):
                  try:
                      data.append(content[addr])
                  except KeyError:
                      #if the HEX file doesn't contain a value for the given address
                      #we "pad" it with 0xFF, which corresponds to the erase value
                      data.append(0xFF)
                  addr+=1
            try:
                if resend >= 3:
                     abort = True
                     break

                self.serial.flushInput()
                self.serial.write(self._create_cmd_message([CMD_WRITE] +map(ord, struct.pack("I", saddr))))
                ret = self.serial.read(1)
                if len(ret) == 1:
                    if struct.unpack("b", ret)[0] != ACK:
                        raise ProgramModeError("Write abort")
                else:
                    raise TimeoutError("Timeout error")
                encdata =data# self._encryptMessage(data)
                self.serial.flushInput()
                self.serial.write(self._create_cmd_message(data))
                ret = self.serial.read(1)
                if len(ret) == 1:
                    if struct.unpack("b", ret)[0] != ACK:
                        raise ProgramModeError("Write abort")
                else:
                    raise TimeoutError("Timeout error")

                yield {"loc": saddr, "resend": resend}
                resend = 0
            except (TimeoutError, ProgramModeError):
                resend +=1

        yield {"success": not abort}


if __name__ == '__main__':
    eraseDone = 0
    
    parser = argparse.ArgumentParser(
        description='Loads a IntelHEX binary file using the custom bootloader described in the "MasteringSTM32 book')

    parser.add_argument('com_port', metavar='com_port_path', type=str,
                        help="Serial port ('/dev/tty.usbxxxxx' for UNIX-like systems or 'COMx' for Windows")

    parser.add_argument('hex_file', metavar='hex_file_path', type=str,
                        help="Path to the IntelHEX file containing the firmware to flash on the target MCU")

    args = parser.parse_args()

    def doErase(arg):
        global eraseDone
        
    
    flasher = STM32Flasher(args.com_port)
    
	
    flasher._sstr_("s")
   
    print("\n\twant to upgrade the firmware?")
    value = input("Please enter yes or No:\n")
    if value == "yes":
        print("Loading %s HEX file...." % args.hex_file)

        
        for e in flasher.writeImage(sys.argv[2]):
            if "saddr" in e:
                print("\tStart address: ", hex(e["saddr"]))
                print("\tEnd address: ", hex(e["eaddr"]))
            if "loc" in e:
                if e["resend"] > 0:
                   end = "\n"
                else:
                    end = ""
                print("\r\tWriting address: %s --- %d" % (hex(e["loc"]), e["resend"]), end=end)
                sys.stdout.flush()

            if "success" in e and e["success"]:
                print("\n\tDone")
                print("\tStart address: ", hex(sad))
                print("\n\Erase last sector ")
                if sad == 0x08020000:
                    flasher.eraseFLASH(0x30)
                if sad == 0x08040000:
                    flasher.eraseFLASH(0x28)		
                time.sleep(1)
                eraseDone = 1				
                print("Erasing Flash memory", end="") 
                while eraseDone == 0:		
                    print(".", end="")
                    sys.stdout.flush()
                    time.sleep(0.5)
                flasher._update(sad)
                print(" Done")				 
            elif "success" in e and not e["success"]:
                print("\n\tFailed to upload firmware")         
            
    print("\n\twant to add write protection?")
    value = input("Please enter yes or No:\n")
    if value == "yes":
        print("\n\enter sector")
        value = input("Please enter sector num\n")
        flasher._write_protection(value)
    else:
        print("\n\ok")
    print("\n\twant to remove write protection?")
    value = input("Please enter yes or No:\n")
    if value == "yes":
        print("\n\enter sector")
        value = input("Please enter sector num\n")
        flasher._Re_write_protection(value)
    else:
        print("\n\ok")				
    flasher._jump(sad)				
			
        
	
