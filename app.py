from flask import Flask, request, jsonify, send_from_directory
import subprocess
import os

app = Flask(__name__, static_folder='static')

current_dir = os.getcwd()

@app.route('/')
def index():
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/execute', methods=['POST'])
def execute():
    global current_dir
    data = request.get_json()
    command = data.get('command', '').strip()

    if not command:
        return jsonify({"error": "No command provided.", "output": ""})

    if command.lower() == 'cls':
        return jsonify({"output": ""})

    if command.lower() == 'help':
        help_msg = (
            "Available commands:\n"
            "- cd [dir]: Change directory\n"
            "- cls: Clear screen\n"
            "- ai: [prompt]: Ask AI\n"
            "- hello: Greeting message\n"
            "- help: Shows a custom help message\n"
            "- ls: List directory contents\n"
            "- pwd: Print working directory\n"
            "- mkdir [dir]: Create a new directory\n"
            "- rm -r [dir]: Remove directory recursively\n"
            "- Any valid Unix shell command\n"
            "- You can also use pipes | and redirection >, >>"
        )
        return jsonify({"output": help_msg})

    if command.lower() == 'hello':
        return jsonify({"output": "Hello, Khushi! Welcome to SmartShell."})

    try:
        # cd command
        if command.startswith('cd '):
            path = command[3:].strip()
            new_dir = os.path.abspath(os.path.join(current_dir, path))
            if os.path.isdir(new_dir):
                current_dir = new_dir
                os.chdir(current_dir)
                return jsonify({"output": f"Changed directory to {current_dir}", "cwd": current_dir})
            else:
                return jsonify({"error": f"cd: no such directory: {path}", "output": ""})

        # ai command
        if command.startswith('ai:'):
            prompt = command[3:].strip()
            llama_cmd = [
                '/home/khushi/llama.cpp/build/bin/llama-run',
                '/home/khushi/llama.cpp/models/mistral-7b-instruct-v0.1.Q4_K_M.gguf',
                prompt
            ]

            result = subprocess.run(llama_cmd, capture_output=True, text=True, timeout=60)
            output = result.stdout.strip()
            return jsonify({"output": output if output else "AI responded with no output."})

        # fallback: shell command
        result = subprocess.run(command, shell=True, capture_output=True, text=True, cwd=current_dir, timeout=10)
        output = result.stdout + result.stderr

        if output.strip() == "":
            return jsonify({"output": "Command executed but no output returned."})

        return jsonify({"output": output, "cwd": current_dir})

    except subprocess.CalledProcessError as e:
        return jsonify({"error": str(e), "output": ""})
    except subprocess.TimeoutExpired:
        return jsonify({"error": "Command timed out.", "output": ""})
    except Exception as e:
        return jsonify({"output": "Command not found. Type 'help' or 'ai: your question' for assistance."})

if __name__ == '__main__':
    app.run(debug=True)
