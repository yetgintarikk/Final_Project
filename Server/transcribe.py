import whisper
import torch
import numpy as np

class Transcriber:
    def __init__(self):
        print("CUDA : " + str(torch.cuda.is_available()))  # check if CUDA is available
        self.model = whisper.load_model("medium", device='cuda') # load model
        # model -> tiny, base, small, medium, large
        print("Model Loaded")

    def transcribeAudioApi(self,data):
        audio_np = np.frombuffer(data, dtype=np.int16).astype(np.float32) / 32768.0  # To use raw data
        result = self.model.transcribe(audio_np , language = 'tr', fp16 = torch.cuda.is_available())
        text = result['text'].strip()
        print(text)
        return text.lower()