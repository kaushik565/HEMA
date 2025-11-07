                      

"""
ACTJv20(RJSR) UART Communication Protocol

The ACTJv20 firmware expects BOTH GPIO handshaking AND UART communication:

1. GPIO Handshaking (RASP_IN_PIC):
   - HIGH = Pi ready for commands  
   - LOW = Pi busy processing

2. UART Protocol:
   - Firmware sends: '20' (scan command with retry), '19' (scan final attempt), '0' (stop)
   - Pi must respond: 'A' (accept/pass), 'R' (reject/fail), 'S' (scanner error)

3. Complete Flow:
   - Firmware: write_rom_rpi(20) → Pi via UART
   - Firmware: wait_busy_rpi() → Check RASP_IN_PIC goes LOW  
   - Pi: Process QR and set RASP_IN_PIC LOW (busy)
   - Pi: Send 'A' or 'R' response via UART
   - Pi: Set RASP_IN_PIC HIGH (ready)
   - Firmware: wait_for_qr() → Receive Pi response
   - Firmware: Advance cartridge based on result
"""

import logging
import serial
import time
import threading
from typing import Optional

from hardware import get_hardware_controller


class ACTJv20UARTProtocol:
    """UART communication protocol for ACTJv20(RJSR) firmware."""
    
    def __init__(self, port="/dev/serial0", baudrate=115200, camera_scanner=None):
        self.logger = logging.getLogger("actj_uart")
        self.hardware = get_hardware_controller()
        self.serial_port = None
        self.port = port
        self.baudrate = baudrate
        self.running = False
        self.listen_thread = None
        
                                
        self.qr_validator = None
        
                                  
        self.camera_scanner = camera_scanner
        
                           
        self._waiting_for_qr = False
        self._scan_start_time = 0
        
    def connect(self):
        """Connect to ACTJv20 UART port."""
        try:
            self.serial_port = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1.0,
                bytesize=8,
                parity='N',
                stopbits=1
            )
            self.logger.info(f"Connected to ACTJv20 on {self.port}")
            return True
        except Exception as e:
            self.logger.error(f"Failed to connect to ACTJv20: {e}")
            return False
    
    def set_qr_validator(self, validator_func):
        """Set QR validation function that returns ('PASS'/'FAIL', mould)."""
        self.qr_validator = validator_func
    
    def start_listening(self):
        """Start listening for ACTJv20 commands."""
        if not self.serial_port:
            if not self.connect():
                return False
                
        self.running = True
        self.listen_thread = threading.Thread(target=self._listen_loop, daemon=True)
        self.listen_thread.start()
        self.logger.info("Started listening for ACTJv20 commands")
        return True
    
    def stop_listening(self):
        """Stop listening for ACTJv20 commands."""
        self.running = False
        if self.listen_thread:
            self.listen_thread.join(timeout=2.0)
        if self.serial_port:
            self.serial_port.close()
            self.serial_port = None
        self.logger.info("Stopped ACTJv20 communication")
    
    def _listen_loop(self):
        """Main listening loop for ACTJv20 commands (BINARY protocol)."""
        while self.running:
            try:
                if self.serial_port and self.serial_port.in_waiting > 0:
                                                                    
                    data = self.serial_port.read(1)
                    if data:
                        cmd_byte = data[0]                             
                        self.logger.debug(f"Received ACTJv20 byte: {cmd_byte} (0x{cmd_byte:02X})")
                        self._handle_command(chr(cmd_byte))                                     
                
                time.sleep(0.01)                                    
                
            except Exception as e:
                self.logger.error(f"Error in ACTJv20 listen loop: {e}")
                time.sleep(0.1)
    
    def _handle_command(self, command):
        """Handle command from ACTJv20 firmware (BINARY protocol)."""
                                                                    
                                                    
        cmd_byte = ord(command) if isinstance(command, str) else command
        
        self.logger.debug(f"ACTJv20 command byte: {cmd_byte} (0x{cmd_byte:02X})")
        
        if cmd_byte == 20:                                  
            self.logger.info("ACTJv20 scan command (20 = 0x14)")
            self._handle_scan_command()
            
        elif cmd_byte == 19:                             
            self.logger.info("ACTJv20 final scan command (19 = 0x13)")
            self._handle_scan_command()
            
        elif cmd_byte == 0:                       
            self.logger.info("ACTJv20 stop command (0 = 0x00)")
            self._handle_stop_command()
            
        else:
            self.logger.warning(f"Unknown ACTJv20 command: {cmd_byte} (0x{cmd_byte:02X})")
    
    def _handle_scan_command(self):
        """Handle QR scan command from ACTJv20."""
        try:
                                                       
            self.hardware.signal_busy_to_firmware()
            self.logger.info("ACTJv20 scan command - signaling BUSY, triggering camera scan")
            
                                                        
            self._waiting_for_qr = True
            self._scan_start_time = time.time()
            
                                                     
            if self.camera_scanner:
                                                     
                self.camera_scanner.on_qr_detected = self.process_qr_input
                self.camera_scanner.start_scanning()
                self.logger.info("Camera scanner triggered")
            else:
                self.logger.warning("No camera scanner available - waiting for manual QR input")
            
                                                
            timeout = 30.0
            while self._waiting_for_qr and (time.time() - self._scan_start_time) < timeout:
                time.sleep(0.1)
            
                                          
            if self.camera_scanner:
                self.camera_scanner.stop_scanning()
            
            if self._waiting_for_qr:
                                              
                self.logger.warning("QR scan timeout - sending scanner error")
                self.serial_port.write(b'S')                 
            
                                            
            time.sleep(0.1)
            self.hardware.signal_ready_to_firmware()
            self.logger.info("ACTJv20 scan complete - signaling READY")
            
        except Exception as e:
            self.logger.error(f"Error handling scan command: {e}")
                                  
            if self.camera_scanner:
                try:
                    self.camera_scanner.stop_scanning()
                except:
                    pass
                                 
            try:
                self.serial_port.write(b'S')                 
                self.hardware.signal_ready_to_firmware()
            except:
                pass
    
    def process_qr_input(self, qr_code):
        """Process QR code input from USB scanner or manual entry.
        
        Returns:
            Tuple of (status, mould) or None if not waiting for QR
        """
        if not self._waiting_for_qr:
            self.logger.debug(f"Received QR {qr_code} but not waiting for input")
            return None
        
        try:
            self.logger.info(f"Processing QR code: {qr_code}")
            
                                                    
            status = "FAIL"
            mould = None
            
            if self.qr_validator:
                status, mould = self.qr_validator(qr_code)
                
                if status == "PASS":
                    response = 'A'          
                    self.logger.info(f"QR validation PASSED ({mould}) - sending Accept")
                else:
                    response = 'R'            
                    self.logger.info(f"QR validation FAILED ({status}) - sending Reject")
            else:
                response = 'A'                                     
                self.logger.warning("No QR validator set - defaulting to Accept")
                status = "PASS"
            
                                                                               
            self.hardware.signal_busy_to_firmware()
            time.sleep(0.1)                                                   
            
                                      
            self.serial_port.write(response.encode('ascii'))
            self.logger.info(f"Sent response to ACTJv20: {response}")
            
                                                                                        
            time.sleep(0.15)                                                  
            
            if response == 'A':
                                                                     
                self.hardware.signal_accept_pulse()
                self.logger.info("Sent ACCEPT GPIO pulse sequence for mechanism plate")
                
            elif response == 'R':
                                                                    
                self.hardware.signal_rejection_pulse()                                    
                self.logger.info("Sent REJECT GPIO pulse sequence for mechanism plate")
                
            else:
                                                   
                self.hardware.signal_ready_to_firmware()
                self.logger.info("Sent ERROR GPIO signal")
            
                                                    
            time.sleep(0.1)                           
            self.hardware.signal_ready_to_firmware()
            
                                    
            self._waiting_for_qr = False
            
                                          
            return (status, mould)
            
        except Exception as e:
            self.logger.error(f"Error processing QR input: {e}")
            try:
                self.serial_port.write(b'S')                 
                self._waiting_for_qr = False
            except:
                pass
            return ("FAIL", None)
    
    def _handle_stop_command(self):
        """Handle stop command from ACTJv20."""
        self.logger.info("ACTJv20 stop command - setting ready state")
        self.hardware.signal_ready_to_firmware()


                                         
_uart_protocol: Optional[ACTJv20UARTProtocol] = None


def get_uart_protocol(camera_scanner=None) -> ACTJv20UARTProtocol:
    """Get singleton UART protocol instance."""
    global _uart_protocol
    if _uart_protocol is None:
        _uart_protocol = ACTJv20UARTProtocol(camera_scanner=camera_scanner)
    elif camera_scanner and not _uart_protocol.camera_scanner:
                                                  
        _uart_protocol.camera_scanner = camera_scanner
    return _uart_protocol


def start_actj_communication(qr_validator_func=None, camera_scanner=None):
    """Start ACTJv20 UART communication with QR validator and camera scanner."""
    protocol = get_uart_protocol(camera_scanner)
    if qr_validator_func:
        protocol.set_qr_validator(qr_validator_func)
    return protocol.start_listening()


def stop_actj_communication():
    """Stop ACTJv20 UART communication."""
    protocol = get_uart_protocol()
    protocol.stop_listening()


if __name__ == "__main__":
                            
    logging.basicConfig(level=logging.DEBUG)
    
    def test_qr_validator(qr_code):
        """Test QR validator function."""
        print(f"Validating QR: {qr_code}")
        return "PASS", "TEST_MOULD"
    
    protocol = ACTJv20UARTProtocol()
    protocol.set_qr_validator(test_qr_validator)
    
    if protocol.start_listening():
        print("ACTJv20 UART protocol test started")
        print("Send commands to test...")
        
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("Stopping...")
            protocol.stop_listening()
    else:
        print("Failed to start ACTJv20 UART protocol")