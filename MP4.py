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

//golden
import tkinter as tk
from tkinter import filedialog, messagebox
from tkinter import ttk
import os
from tkinter.scrolledtext import ScrolledText
import subprocess
import re

class WidgetFrame(tk.Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self.grid(padx=10, pady=10, sticky="nsew")

        # Frame for Source Code Build Section
        self.create_build_frame()

        # Frame for Golden Data Generation Section
        self.create_golden_data_frame()

        # Common Log Box
        self.create_log_box()

        # Configure row and column weights for resizing
        self.grid_rowconfigure(2, weight=1)
        self.grid_columnconfigure(1, weight=1)

    def create_build_frame(self):
        build_frame = tk.LabelFrame(self, text="Source Code Build", padx=10, pady=10)
        build_frame.grid(row=0, column=0, padx=10, pady=10, sticky="nsew", columnspan=3)

        self.folder_path_label = tk.Label(build_frame, text="Selected folder:", bg="silver")
        self.folder_path_label.grid(row=0, column=0, pady=5, sticky="w")

        self.folder_path_entry = tk.Entry(build_frame, width=50)
        self.folder_path_entry.grid(row=0, column=1, pady=5, padx=5, sticky="ew")

        self.browse_button = tk.Button(build_frame, text="Browse", command=self.browse_folder, bg="silver")
        self.browse_button.grid(row=0, column=2, pady=5, padx=5, sticky="ew")

        self.build_button = tk.Button(build_frame, text="Build", command=self.build_source_code, bg="silver")
        self.build_button.grid(row=1, column=2, pady=5, padx=5, sticky="ew")

    def create_golden_data_frame(self):
        golden_data_frame = tk.LabelFrame(self, text="Golden Data Generation", padx=10, pady=10)
        golden_data_frame.grid(row=1, column=0, padx=10, pady=10, sticky="nsew", columnspan=3)

        self.input_label = tk.Label(golden_data_frame, text="Decoder path:", bg="silver")
        self.input_label.grid(row=0, column=0, pady=5, sticky="w", columnspan=3)

        self.decoder_data_entry = tk.Entry(golden_data_frame, width=50)
        self.decoder_data_entry.grid(row=0, column=1, pady=5, padx=5, sticky="ew")

        self.input_browse_button = tk.Button(golden_data_frame, text="Browse", command=self.browse_decoder_data, bg="silver")
        self.input_browse_button.grid(row=0, column=2, pady=5, padx=5, sticky="ew")

        self.referance_file_label = tk.Label(golden_data_frame, text="Reference File:", bg="silver")
        self.referance_file_label.grid(row=1, column=0, pady=5, sticky="w")

        self.referane_file_entry = tk.Entry(golden_data_frame, width=50)
        self.referane_file_entry.grid(row=1, column=1, pady=5, padx=5, sticky="ew")

        self.output_browse_button = tk.Button(golden_data_frame, text="Browse", command=self.browse_referance_file, bg="silver")
        self.output_browse_button.grid(row=1, column=2, pady=5, padx=5, sticky="ew")

        self.config_label = tk.Label(golden_data_frame, text="Config File Path:", bg="silver")
        self.config_label.grid(row=2, column=0, pady=5, sticky="w")

        self.config_entry = tk.Entry(golden_data_frame, width=50)
        self.config_entry.grid(row=2, column=1, pady=5, padx=5, sticky="ew")

        self.config_browse_button = tk.Button(golden_data_frame, text="Browse", command=self.browse_config_file, bg="silver")
        self.config_browse_button.grid(row=2, column=2, pady=5, padx=5, sticky="ew")

        self.testmaterial_label = tk.Label(golden_data_frame, text="Test Material:", bg="silver")
        self.testmaterial_label.grid(row=3, column=0, pady=5, sticky="w")

        self.testmaterial_entry = tk.Entry(golden_data_frame, width=50)
        self.testmaterial_entry.grid(row=3, column=1, pady=5, padx=5, sticky="ew")

        self.testmaterial_browse_button = tk.Button(golden_data_frame, text="Browse", command=self.browse_testmaterial, bg="silver")
        self.testmaterial_browse_button.grid(row=3, column=2, pady=5, padx=5, sticky="ew")

        self.testvectors_label = tk.Label(golden_data_frame, text="Test Vectors:", bg="silver")
        self.testvectors_label.grid(row=4, column=0, pady=5, sticky="w")

        self.testvectors_combobox = ttk.Combobox(golden_data_frame, state="readonly")
        self.testvectors_combobox.grid(row=4, column=1, pady=5, padx=5, sticky="nsew")

        self.generate_button = tk.Button(golden_data_frame, text="Generate Golden Data", command=self.generate_golden_data, bg="silver")
        self.generate_button.grid(row=5, column=2, pady=10)

    def create_log_box(self):
        self.log_box = ScrolledText(self, wrap=tk.WORD, height=10, width=70, bg="black", fg="white")
        self.log_box.grid(row=2, column=0, columnspan=3, pady=10, sticky="nsew")

        self.clear_log_button = tk.Button(self, text="Clear Log", command=self.clear_log, bg="silver")
        self.clear_log_button.grid(row=3, column=0, columnspan=3, pady=10)

    def browse_folder(self):
        folder_selected = filedialog.askdirectory()
        if folder_selected:
            self.folder_path_entry.delete(0, tk.END)
            self.folder_path_entry.insert(0, folder_selected)

    def browse_decoder_data(self):
        decoder_file = filedialog.askopenfile(filetypes=[("All Files", "*.*")])
        if decoder_file:
            self.decoder_data_entry.delete(0, tk.END)
            self.decoder_data_entry.insert(0, decoder_file.name)

    def browse_referance_file(self):
        output_file = filedialog.askopenfile(filetypes=[("All Files", "*.*")])
        if output_file:
            self.referane_file_entry.delete(0, tk.END)
            self.referane_file_entry.insert(0, output_file.name)

    def browse_config_file(self):
        config_file = filedialog.askopenfile(filetypes=[("Config Files", "*.*"), ("All Files", "*.*")])
        if config_file:
            self.config_entry.delete(0, tk.END)
            self.config_entry.insert(0, config_file.name)

    def browse_testmaterial(self):
        test_material_folder = filedialog.askdirectory()
        if test_material_folder:
            self.testmaterial_entry.delete(0, tk.END)
            self.testmaterial_entry.insert(0, test_material_folder)

            subdirectories = next(os.walk(test_material_folder))[1]
            if subdirectories:
                self.testvectors_combobox['values'] = subdirectories
                self.testvectors_combobox.set('Select Test Vector')
            else:
                self.testvectors_combobox['values'] = ["No test vectors found"]
                self.testvectors_combobox.set("No test vectors found")

    def clear_log(self):
        self.log_box.delete(1.0, tk.END)

    def generate_golden_data(self):
        # Get the paths from the entries
        decoder_file = self.decoder_data_entry.get()
        reference_file = self.referane_file_entry.get()
        config_file = self.config_entry.get()
        test_material_path = self.testmaterial_entry.get()
        test_vector = self.testvectors_combobox.get()

        if not all([decoder_file, reference_file, config_file, test_material_path, test_vector]):
            messagebox.showerror("Error", "Please ensure all fields are filled in correctly.")
            return

        try:
            # Process the reference file to extract details
            extracted_portions = process_reference_file(reference_file)

            # Use the extracted details for the command
            for i, portion in enumerate(extracted_portions, start=1):
                self.log_box.insert(tk.END, f"Extracted portion for test vector {i}: {portion}\n")
                self.log_box.see(tk.END)

            # Example command with placeholders
            resolution = extracted_portions[2]
            command = f"/home/bharani.vidyaakar/Test_Tools/scripts/fxp_decoder && python {decoder_file} --input {test_material_path}/{test_vector} --r {reference_file} --config {config_file}"

            self.log_box.insert(tk.END, f"Generated command: {command}\n")
            self.log_box.see(tk.END)

            # Run the generated command
            process = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            self.log_box.insert(tk.END, process.stdout)
            self.log_box.insert(tk.END, process.stderr)
            self.log_box.see(tk.END)

        except Exception as e:
            self.log_box.insert(tk.END, f"Error: {e}\n")
            self.log_box.see(tk.END)
            messagebox.showerror("Error", str(e))

    def build_source_code(self):
        folder_path = self.folder_path_entry.get()
        if not os.path.isdir(folder_path):
            messagebox.showerror("Error", "Please select a valid source code folder")
            return

        cmakelists_path = os.path.join(folder_path, "CMakeLists.txt")
        if not os.path.isfile(cmakelists_path):
            messagebox.showerror("Error", f"CMakeLists.txt not found in {folder_path}")
            return

        try:
            self.log_box.delete(1.0, tk.END)
            self.log_box.insert(tk.END, f"Building source code in folder: {folder_path}\n")
            self.log_box.see(tk.END)

            self.log_box.insert(tk.END, "Running: cmake -Bbuild\n")
            self.log_box.see(tk.END)
            result = subprocess.run(["cmake", "-Bbuild"], cwd=folder_path, capture_output=True, text=True)
            self.log_box.insert(tk.END, result.stdout)
            self.log_box.insert(tk.END, result.stderr)
            self.log_box.see(tk.END)

            self.log_box.insert(tk.END, "Running: cmake --build build\n")
            self.log_box.see(tk.END)
            result = subprocess.run(["cmake", "--build", "build"], cwd=folder_path, capture_output=True, text=True)
            self.log_box.insert(tk.END, result.stdout)
            self.log_box.insert(tk.END, result.stderr)
            self.log_box.see(tk.END)

            messagebox.showinfo("Success", "Source code built successfully!")
        except Exception as e:
            messagebox.showerror("Error", f"Error during build: {str(e)}")

# Helper functions for reference file processing
def process_reference_file(command):
    # Dummy implementation for extracting details from a file
    with open(reference_file_path, 'r') as file:
            commands = file.readlines()

    extracted_details = []

    for command in commands:
        cline = extract_details_from_referance_file(command)
        if cline:
                extracted_details.append(cline)

        return extracted_details


    # Example usage
    reference_file_path = 'test_vectors.txt'  # Replace with the path to your reference file

    extracted_portions = process_reference_file(reference_file_path)

    # Display the extracted details for each test vector
    for i, portion in enumerate(extracted_portions, start=1):
        print(f"Extracted portion for test vector {i}: {portion}")

def extract_details_from_referance_file(command):
    start = command.find('-r')
    end = command.find('-fps')

    if start != -1 and end != -1:
        return command[start:end].strip()
    else:
        return None

if __name__ == "__main__":
    root = tk.Tk()
    root.title("Golden Data Generator")
    root.geometry("800x600")

    app = WidgetFrame(root)
    app.pack(fill="both", expand=True)

    root.mainloop()
//Upated
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
import os
import re
from tkinter import ttk
import subprocess


class MP4GeneratorApp(tk.Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self.configure(bg='silver')
        self.valid_profiles = {}
        self.valid_track_numbers = {}
        self.test_vectors = []
        self.selected_test_vector = tk.StringVar(value="Select Test Vector")
        self.test_material_folder_path = tk.StringVar()
        self.batch_file_loaded = False  # Flag to check if the batch file is loaded
        self.create_widgets()

    def create_widgets(self):
        self.exe_path = tk.StringVar()

        # .exe file selection
        tk.Label(self, text="Select .exe file:").grid(row=0, column=0, padx=10, pady=10, sticky='w')
        tk.Entry(self, textvariable=self.exe_path, width=50).grid(row=0, column=1, padx=10, pady=10, sticky='ew')
        tk.Button(self, text="Browse", command=self.browse_exe).grid(row=0, column=2, padx=10, pady=10)

        # Test Material folder selection
        tk.Label(self, text=" Selesct Test Material:").grid(row=1, column=0, padx=10, pady=10, sticky='w')
        tk.Entry(self, textvariable=self.test_material_folder_path, width=50).grid(row=1, column=1, padx=10, pady=10, sticky='ew')
        tk.Button(self, text="Browse", command=self.browse_test_material_folder).grid(row=1, column=2, padx=10, pady=10)

        # Button to load the batch file
        tk.Button(self, text="Load Batch File", command=self.load_batch_file).grid(row=2, column=0, padx=10, pady=10)

        # Test Vector selection via scrollable dropdown
        #tk.Label(self, text="Select Test Vector:").grid(row=2, column=0, padx=10, pady=10, sticky='w')
        self.vector_menu = tk.OptionMenu(self, self.selected_test_vector, self.test_vectors)
        self.vector_menu.grid(row=2, column=1, padx=10, pady=10, sticky='ew')
        #self.selected_test_vector.trace('w', self.on_test_vector_change)

        # Generate MP4 button
        tk.Button(self, text="Generate MP4", command=self.generate_mp4).grid(row=2, column=2,  padx=10, pady=10)

        # Log box
        self.log_box = scrolledtext.ScrolledText(self, wrap=tk.WORD, bg="black", fg="white", font=('Arial', 10))
        self.log_box.grid(row=3, column=0, columnspan=3, padx=10, pady=10, sticky='nsew')

        # Clear Log button
        tk.Button(self, text="Clear Log", command=self.clear_log).grid(row=4, column=0, columnspan=3, pady=10)

        # Configure row and column weights
        self.grid_rowconfigure(3, weight=1)
        self.grid_columnconfigure(1, weight=1)
        self.grid_columnconfigure(2, weight=0)

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
        # Open file dialog to select the batch file
        file_path = filedialog.askopenfilename(filetypes=[("Batch files", "*.bat"), ("All files", "* *")])
        
        # If no file is selected, return
        if not file_path:
            return
        
        # Parse the batch file and store details for all test vectors
        self.parse_batch_file(file_path)
        
        # Log success message
        self.log_box.insert(tk.END, "Batch file loaded successfully. You can now select test vectors and generate the MP4 file.\n")
        self.batch_file_loaded = True

    def parse_batch_file(self, file_path):
        self.valid_profiles.clear()
        self.valid_track_numbers.clear()
        
        with open(file_path, 'r') as file:
            lines = file.readlines()
            
            # Iterate over each line and look for test vector details, including variations like 1a, 1b, etc.
            for line in lines:
                test_vector_match = re.search(r'%TEST_VECTORS_ROOT%\\(\d+[a-z]?)', line)
                
                if test_vector_match:
                    test_vector = test_vector_match.group(1)  # Capture test vector with suffix if present
                    
                    # Extract track number and DV profile for the chosen test vector
                    track_number_match = re.search(r'--num-track\s+(\d+)', line)
                    profile_id_match = re.search(r'--dv-profile\s+([^\s]+)', line)

                    if profile_id_match and track_number_match:
                        self.valid_profiles[test_vector] = profile_id_match.group(1)
                        self.valid_track_numbers[test_vector] = track_number_match.group(1)

    def generate_mp4(self):
        exe_file = self.exe_path.get()
        test_vector = self.selected_test_vector.get()
        
        # Handle suffixes like 1a, 1b, 1c by checking for variations
        dv_profile_value = self.valid_profiles.get(test_vector, '')
        track_number = self.valid_track_numbers.get(test_vector, '')

        if not exe_file or not test_vector or not dv_profile_value or not track_number:
            self.log_box.insert(tk.END, "Please provide all required fields.\n")
            return

        new_dir = os.path.dirname(exe_file)
        input_folder = os.path.join(self.test_material_folder_path.get(), test_vector)
        
        # Search for .265, .ivf, or .264 files, including subdirectories and variations in file names
        new_file = None
        for root, dirs, files in os.walk(input_folder):
            for f in files:
                if f.endswith(".265") or f.endswith(".ivf") or f.endswith(".264"):
                    new_file = os.path.join(root, f)
                    break
            if new_file:
                break

        if not new_file:
            self.log_box.insert(tk.END, f"No valid video files found in the folder for {test_vector} or its subdirectories.\n")
            return

        command = f"cd {new_dir} && {exe_file} --input-ves {new_file} --dv-profile {dv_profile_value} --num-track {track_number} "
        print(command)
        
        self.log_box.insert(tk.END, f"Executing command: {command}\n")

        try:
            process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            stdout, stderr = process.communicate()

            if stdout:
                self.log_box.insert(tk.END, "Output:\n" + stdout + "\n")
            if stderr:
                self.log_box.insert(tk.END, "Errors:\n" + stderr + "\n")

            self.log_box.insert(tk.END, "MP4 generation completed.\n")
        except Exception as e:
            self.log_box.insert(tk.END, f"An error occurred: {e}\n")

    def clear_log(self):
        self.log_box.delete('1.0', tk.END)
