import requests
import json
import pyttsx3
import serial
from scripts_map import SCRIPTS

OLLAMA_SERVER = "http://192.168.0.22:11434"
MAX_HISTORY = 10
conversation_history: list[str] = []
SYSTEM_PROMPT = f"""
    You are a freindly retro robot assistant named Clanker.
    You are being fed through a text-to-speech program so omit any onomatopoeia sounds.
    When the user gives a command, do TWO things:
    1. Speak back in a fun retro robot style.
    2. Output a JSON object on a new line with the action, like:
        {{ "action": "move_backward}}

    Available actions: {list(SCRIPTS.keys())}

    If no action applies, use {{ "action": "none" }}
    """

ser = serial.Serial(port="/dev/ttyACM0", baudrate=9600, timeout=1)  # adjust port/baudrate

def trim_history():
    global conversation_history
    if len(conversation_history) > MAX_HISTORY:
        conversation_history = conversation_history[-MAX_HISTORY:]


def query_ollama(user_input):
    # --- send wakeup if serial is available ---
    if ser and ser.is_open:
        ser.write(b"wakeup\n")
    else:
        print("⚠️ Serial not open, skipping wakeup")

    conversation_history.append({"role": "user", "content": user_input})

    trim_history()

    prompt = SYSTEM_PROMPT + "\n"
    for msg in conversation_history:
        prompt += f'{msg["role"]}: {msg["content"]}\n'
    prompt += "assistant: "

    response = requests.post(
        f"{OLLAMA_SERVER}/api/generate",
        headers={"Content-Type": "application/json"},
        json={"model": "gemma3:4b", "prompt": prompt, "stream": False},
    )

    reply = response.json()["response"]
    conversation_history.append({"role": "assistant", "content": reply})

    return reply


def handle_response(reply):
    lines = reply.strip().splitlines()
    text_part = "\n".join(line for line in lines if not line.strip().startswith("{"))
    json_part = next((line for line in lines if line.strip().startswith("{")), None)

    print("Robot says: ", text_part)
    speak(text_part)

    if json_part:
        try:
            action = json.loads(json_part).get("action")
            if action in SCRIPTS:
                SCRIPTS[action]()
            else:
                print("No valid action")
        except json.JSONDecodeError:
            print("Could not parse action")


def speak(text):
    engine.say(text)
    engine.runAndWait()


if __name__ == "__main__":
    engine = pyttsx3.init()
    try:
        while True:
            user_input = input("You: ")
            if user_input.lower() in ["quit", "exit"]:
                break
            reply = query_ollama(user_input)
            handle_response(reply)
    finally:
        ser.close()