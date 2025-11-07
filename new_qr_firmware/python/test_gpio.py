                      
"""Test GPIO handshaking with PIC."""

try:
    import RPi.GPIO as GPIO
    
    print("=== GPIO Handshake Test ===")
    print("Testing RASP_IN_PIC (GPIO 12) communication")
    print("")
    
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(12, GPIO.OUT)
    
                           
    print("1. Setting GPIO 12 LOW (Pi BUSY)...")
    GPIO.output(12, GPIO.LOW)
    input("   Press ENTER to continue...")
    
                      
    print("2. Setting GPIO 12 HIGH (Pi READY)...")
    GPIO.output(12, GPIO.HIGH)
    print("   → PIC should now see Pi as READY")
    print("   → Press START button on PIC now")
    input("   Press ENTER after testing...")
    
                 
    print("3. Toggling GPIO 12 (simulating busy/ready cycle)...")
    for i in range(5):
        print(f"   Cycle {i+1}: HIGH (ready)...")
        GPIO.output(12, GPIO.HIGH)
        import time
        time.sleep(1)
        print(f"   Cycle {i+1}: LOW (busy)...")
        GPIO.output(12, GPIO.LOW)
        time.sleep(1)
    
                    
    GPIO.output(12, GPIO.HIGH)
    print("\n✓ GPIO test complete")
    print("  Final state: HIGH (ready)")
    
    GPIO.cleanup()
    
except Exception as e:
    print(f"ERROR: {e}")
    print("\nCheck:")
    print("1. Are you running as root? (sudo python3 test_gpio.py)")
    print("2. Is GPIO 12 physically connected to PIC RB6?")
    print("3. Is there a common ground between Pi and PIC?")
