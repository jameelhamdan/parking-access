from django.core.management.base import BaseCommand, CommandError

from parking.models import ParkingSession as PS
from django.utils import timezone

import os
import uuid

from PIL import Image
import pytesseract

import serial
import time

tessdata_dir_config = "/usr/share/tesseract-ocr/4.00/tessdata"
USB_PORT = "/dev/ttyUSB0"

# STATEFUL VARIABLES
STATE_IDLE = "I"
STATE_WAITING_PAYMENT = "P"


class Command(BaseCommand):
    required_total_balance = 0.0
    state = STATE_IDLE

    def __init__(self, *args, **kwargs):
        super(Command, self).__init__(*args, **kwargs)

    def change_state(self, new_state):
        self.state = new_state
        print(new_state)

    def listen_arduino(self, usb):
        response = usb.readline()
        if not response:
            return

        response = response.decode("utf-8")
        print(response)

        if response.startswith("NEW_BALANCE: "):
            t = response.strip("NEW_BALANCE: ")
            total_balance = float(t.split(",")[0])
            new_coin = float(t.split(",")[1])

            if total_balance >= self.required_total_balance:
                # FINSIHED
                usb.write("MESSAGE: FINISHED PAYING".encode())
                time.sleep(3)
                usb.write("RESET_MESSAGE".encode())
                time.sleep(3)
                self.change_state(STATE_IDLE)

            else:
                message = "MESSAGE: Left %s" % (float(self.required_total_balance) - float(total_balance))
                usb.write(message.encode())
                time.sleep(1)

    def capture_image(self, usb):
        # capture image
        random_string = uuid.uuid4().hex[:12]
        new_file_name = f'/home/pi/tmp/{random_string}.jpg'
        command_string = f'fswebcam {new_file_name}'
        os.system(command_string)

        # new_file_name = f'/home/pi/tmp/test.png'

        license_plate = pytesseract.image_to_string(
            Image.open(new_file_name),
            lang='eng',
            config=tessdata_dir_config,
        ).strip()

        print(f'LICENSE "{license_plate}" FOUND')
        if not license_plate or license_plate == '':
            print('NO LICENSE FOUND')
            return False

        try:
            ps = PS.objects.filter(finished_on__isnull=True).get(license_plate__iexact=license_plate)
            ps.finished_on = timezone.now()
            ps.save()
            time.sleep(2)
            self.change_state(STATE_WAITING_PAYMENT)
            self.required_total_balance = ps.price()
            message = "MESSAGE: CREDIT %s" % float(self.required_total_balance)
            usb.write(message.encode())

            print("PAYING FOR SESSION")

        except PS.DoesNotExist:
            ps = PS(license_plate=license_plate)
            ps.save()
            usb.write("NEW_SESSION".encode())
            time.sleep(1)
            print("NEW SESSION")

        return True

    def handle(self, *args, **options):
        try:
            usb = serial.Serial(USB_PORT, 9600, timeout=2)
            usb.flush()
        except Exception as e:
            print("ERROR - Could not open USB serial port.  Please check your port name and permissions.")
            raise e

        print("CONNECTING")
        time.sleep(2)
        print("Listening ...")
        message = "PING"
        print("PING")

        try:
            usb.write(message.encode())
            time.sleep(5)
            while True:
                if self.state == STATE_IDLE:

                    accepted = self.capture_image(usb)
                    if not accepted:
                        time.sleep(10)
                    else:
                        time.sleep(2)

                elif self.state == STATE_WAITING_PAYMENT:
                    # Listen on Arduino
                    self.listen_arduino(usb)

        except KeyboardInterrupt:
            usb.close()
