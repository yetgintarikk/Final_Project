import json

class Parser:
    def __init__(self) -> None:
        pass

    def parse(self,text):
        f = open("configs/commands.json", encoding="utf-8")
        self.data = json.load(f)
        f.close()

        for item in self.data["commands"]:
            for jsontext in item["text"]:
                if jsontext in text:
                    return [item["tag"], item["type"], item["val"]]
                    
        return []