import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
import os
import re
import subprocess

class MP4GeneratorApp(tk.Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self.configure(bg='silver')
        self.test_vectors_info = {}  # Dictionary to store test vector, profile, and track info
        self.selected_test_vector = tk.StringVar(value="Select Test Vector")
        self.test_material_folder_path = tk.StringVar()
        self.create_widgets()

    def create_widgets(self):
        self.exe_path = tk.StringVar()

        # .exe file selection
        tk.Label(self, text="Select .exe file:").grid(row=0, column=0, padx=10, pady=10, sticky='w')
        tk.Entry(self, textvariable=self.exe_path, width=50).grid(row=0, column=1, padx=10, pady=10, sticky='ew')
        tk.Button(self, text="Browse", command=self.browse_exe).grid(row=0, column=2, padx=10, pady=10)

        # Test Material folder selection
        tk.Label(self, text="Select Test Material Folder:").grid(row=1, column=0, padx=10, pady=10, sticky='w')
        tk.Entry(self, textvariable=self.test_material_folder_path, width=50).grid(row=1, column=1, padx=10, pady=10, sticky='ew')
        tk.Button(self, text="Browse", command=self.browse_test_material_folder).grid(row=1, column=2, padx=10, pady=10)

        # Test Vector selection via scrollable dropdown
        tk.Label(self, text="Select Test Vector:").grid(row=2, column=0, padx=10, pady=10, sticky='w')
        self.vector_menu = tk.OptionMenu(self, self.selected_test_vector, [])
        self.vector_menu.grid(row=2, column=1, padx=10, pady=10, sticky='ew')

        # Button to load the batch file
        tk.Button(self, text="Load Batch File", command=self.load_batch_file).grid(row=3, column=0, columnspan=3, padx=10, pady=10)

        # Generate MP4 button
        tk.Button(self, text="Generate MP4", command=self.generate_mp4).grid(row=4, column=0, columnspan=3, padx=10, pady=10)

        # Log box
        self.log_box = scrolledtext.ScrolledText(self, wrap=tk.WORD, bg="black", fg="white", font=('Arial', 10))
        self.log_box.grid(row=5, column=0, columnspan=3, padx=10, pady=10, sticky='nsew')

        self.grid_rowconfigure(6, weight=1)
        self.grid_columnconfigure(1, weight=1)

        # Clear Log button
        tk.Button(self, text="Clear Log", command=self.clear_log).grid(row=6, column=0, columnspan=3, padx=10, pady=10)

    def browse_exe(self):
        self.exe_path.set(filedialog.askopenfilename(filetypes=[("Executable files", "*.exe")]))

    def browse_test_material_folder(self):
        folder_path = filedialog.askdirectory(title="Select Test Material Folder")
        if folder_path:
            self.test_material_folder_path.set(folder_path)
            self.load_test_vectors()

    def load_test_vectors(self):
        folder_path = self.test_material_folder_path.get()
        if not folder_path:
            messagebox.showwarning("Input Required", "Please select a Test Material Folder first.")
            return
        
        # Load all files in the Test Material folder as test vectors
        self.test_vectors = os.listdir(folder_path)
        if self.test_vectors:
            self.selected_test_vector.set(self.test_vectors[0])  # Set default selection
            menu = self.vector_menu["menu"]
            menu.delete(0, "end")
            for vector in self.test_vectors:
                menu.add_command(label=vector, command=lambda value=vector: self.selected_test_vector.set(value))
        else:
            messagebox.showwarning("No Test Vectors Found", "No files were found in the selected folder.")

    def load_batch_file(self):
        file_path = filedialog.askopenfilename(filetypes=[("Batch files", "*.bat"), ("All files", "*.*")])
        if not file_path:
            return

        self.parse_batch_file(file_path)
        self.log_box.insert(tk.END, "Batch file loaded successfully. You can now generate the MP4 file.\n")

    def parse_batch_file(self, file_path):
        self.test_vectors_info.clear()
        with open(file_path, 'r') as file:
            lines = file.readlines()
            for line in lines:
                test_vector_match = re.search(r'--input-ves\s+%TEST_VECTORS_ROOT%\\(\d+)', line)
                profile_id_match = re.search(r'--dv-profile\s+(\d+)', line)
                track_number_match = re.search(r'--num-track\s+(\d+)', line)

                if test_vector_match and profile_id_match and track_number_match:
                    test_vector = test_vector_match.group(1)
                    profile_id = profile_id_match.group(1)
                    track_number = track_number_match.group(1)
                    self.test_vectors_info[test_vector] = (profile_id, track_number)

        if not self.test_vectors_info:
            messagebox.showerror("Error", "No valid test vectors found in the batch file.")
            self.log_box.insert(tk.END, "Failed to load valid test vectors from the batch file.\n")

    def generate_mp4(self):
        exe_file = self.exe_path.get()
        selected_test_vector = self.selected_test_vector.get()

        if not exe_file or not selected_test_vector or selected_test_vector not in self.test_vectors_info:
            self.log_box.insert(tk.END, "Please provide all required fields and select a valid test vector.\n")
            return

        dv_profile_value, track_number = self.test_vectors_info[selected_test_vector]
        input_file = os.path.join(self.test_material_folder_path.get(), selected_test_vector)

        if not os.path.isfile(input_file):
            self.log_box.insert(tk.END, f"Test vector {selected_test_vector} not found in the Test Material folder.\n")
            return

        command = f"cd {os.path.dirname(exe_file)} && {exe_file} --input-ves {input_file} --dv-profile {dv_profile_value} --num-track {track_number}"
        self.log_box.insert(tk.END, f"Executing command for Test Vector {selected_test_vector}: {command}\n")

        try:
            process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            stdout, stderr = process.communicate()

            if stdout:
                self.log_box.insert(tk.END, "Output:\n" + stdout + "\n")
            if stderr:
                self.log_box.insert(tk.END, "Errors:\n" + stderr + "\n")

            self.log_box.insert(tk.END, f"MP4 generation for Test Vector {selected_test_vector} completed.\n")
        except Exception as e:
            self.log_box.insert(tk.END, f"An error occurred: {e}\n")

    def clear_log(self):
        self.log_box.delete('1.0', tk.END)

# To run the application, you would create an instance of MP4GeneratorApp inside a Tkinter root window:
if __name__ == "__main__":
    root = tk.Tk()
    root.title("MP4 Generator")
    app = MP4GeneratorApp(root)
    app.pack(fill="both", expand=True)
    root.mainloop()