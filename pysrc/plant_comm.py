import serial, os, pickle
from time import sleep
from datetime import datetime, timedelta
import numpy as np
import json

save_file = "plant_data.json"

class Plant:
    max_length = 16
    plant_id = 0

    def __init__(self, name: str, watering_period: timedelta, latin_name: str, water_amount: int):
        self.watering_period = watering_period
        self.last_watering = datetime.now() - timedelta(days=5)
        self.name = name
        self.latin_name = latin_name
        self.water_amount = water_amount
        self.id = Plant.plant_id
        Plant.plant_id += 1

    def get_comm_msg(self):
        msg = []
        msg.append(self.name[:Plant.max_length])
        msg.append(self.id)
        msg.append(self.water_amount)
        msg.append(self.get_notification_code())
        msg = [str(i) for i in msg]
        return msg

    def get_urgency(self):
        if (datetime.now() - self.last_watering) > self.watering_period:
            return (datetime.now() - self.last_watering) / self.watering_period
        else:
            return 0

    def get_notification_code(self):
        urgency = self.get_urgency()
        if urgency >= 2:
            return 2
        elif urgency >= 1:
            return 1
        else:
            return 0

    def water(self):
        self.last_watering = datetime.now()

    def to_dict(self):
        s = dict()
        s["watering_period"] =  self.watering_period.total_seconds()
        s["last_watering"] = self.last_watering.isoformat()
        s["name"] = self.name
        s["latin_name"] = self.latin_name
        s["water_amount"] = self.water_amount
        s["id"] = self.id
        return s

    @staticmethod
    def from_dict(d):
        p = Plant(name=d["name"],
                  watering_period=timedelta(seconds=d["watering_period"]),
                  latin_name=d["latin_name"],
                  water_amount=d["water_amount"])
        p.last_watering = datetime.fromisoformat(d["last_watering"])
        p.id = d["id"]
        return p

if __name__ == "__main__":
    # read plant from file
    if os.path.isfile(save_file):
        data = json.load(open(save_file, "r"))
    else:
        data = []
        data.append(Plant(name="Test", latin_name="Lat. test", watering_period=timedelta(days=2), water_amount=25))
        data.append(Plant(name="Test2", latin_name="Lat. test2", watering_period=timedelta(days=1), water_amount=100))
        data.append(Plant(name="Test3", latin_name="Lat. test3", watering_period=timedelta(days=0, hours=12), water_amount=50))
        data = [i.to_dict() for i in data]
        json.dump(data, open(save_file, "w"), indent=4, sort_keys=True)
        data = json.load(open(save_file, "r"))
    data = [Plant.from_dict(i) for i in data]
    Plant.plant_id = max([i.id for i in data]) + 1


    ser = serial.Serial("/dev/ttyACM0", 9600)
    sent = False
    message_displayed = False

    try:
        while 1:
            # check plant status
            if not message_displayed:
                plants_to_water = []
                for p in data:
                    if (datetime.now() - p.last_watering) > p.watering_period:
                        plants_to_water.append(p)

                # sort from most to leas needed
                urgencies = [p.get_urgency() for p in plants_to_water]
                idx = np.flip(np.argsort(urgencies, ))
                plants_to_water = [plants_to_water[i] for i in idx]
                urgencies = [p.get_urgency() for p in plants_to_water]

                # send message if needed
                if len(plants_to_water) > 0:
                    plant = plants_to_water[0]
                    message = plant.get_comm_msg()
                    print((",".join(message) + "\n").encode("utf-8"))
                    ser.write((",".join(message)+"\n").encode("utf-8"))
                    message_displayed = True

            # check response
            if ser.in_waiting:
                plant_id = int(ser.readline().decode("utf-8").strip())
                plant = None
                for p in data:
                    if p.id == plant_id:
                        plant = p
                        print("Watering", plant_id)
                        break
                plant.water()
                message_displayed = False

                # save update data
                save_data = [i.to_dict() for i in data]
                json.dump(save_data, open(save_file, "w"), indent=4, sort_keys=True)

            # sleep one second to lower CPU load
            sleep(1)

    finally:
        ser.close()
