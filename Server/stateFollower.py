import json

class StateFollower:
    def castCommand(self, command):
        if len(command) > 0:
            with open('configs/pins.json', 'r') as pins:
                data = json.load(pins)

            for item in data["pins"]:
                if command[0] == item["tag"]:
                    if(command[1] == "set"):
                        item["value"] = command[2]
                    elif (command[1] == "add"):
                        item["value"] = self.__getNewVal(item["value"], item["type"], command[2])
                    retval = (item["type"] + chr(item["pin"]) + chr(item["value"]) + chr(item["channel"]))

            with open('configs/pins.json', 'w') as dosya:
                json.dump(data, dosya)

            return retval

        else:
            return "xxxx"

    def castServerCommand(self, cmd):
        tagAndvalue = cmd.split("|")
        with open('configs/pins.json', 'r') as pins:
             data = json.load(pins)
        for item in data["pins"]:
            if item["tag"] == tagAndvalue[0]:
                item["value"] =  int(tagAndvalue[1])
                retval = (item["type"] + chr(item["pin"]) + chr(item["value"]) + chr(item["channel"]))
                break
        with open('configs/pins.json', 'w') as dosya:
            json.dump(data, dosya)
        return retval

             

    def __getNewVal(self, value, type, change):
        value += change

        # Servo value should be between [0,180]
        if(type == "s"):
            if (value > 180):
                value = 180
            elif (value < 0):
                value = 0

        # Motor value should be between [0,100]
        elif(type == "m"):
            if (value > 100):
                value = 100
            elif (value < 0):
                value = 0

        return value
    
    def sync_states(self):
        with open('configs/pins.json', 'r') as pins:
             data = json.load(pins)

        all_commands = []
        for item in data["pins"]:
            all_commands.append(item["tag"] + "|" + str(item["value"]))

        return all_commands