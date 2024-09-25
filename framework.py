import tkinter as tk
from tkinter import filedialog

def on_button_click():
    label.config(text="Button Clicked!")

def browse_file():
    file_path = filedialog.askopenfilename(title="Select a File", filetypes=(("Text files", "*.txt"),("All files","*.*")))
    if file_path:
        label.config(text=f"Selected File:{file_path}")

root = tk.Tk()
root.title("Simple Tkinter App for Bharani")
root.geometry("500x500")


root.config(bg="lightblue")
           
label = tk.Label(root, text="Hello, Tkinter!",bg="lightyellow",fg="black")
label.pack(pady=10)

button = tk.Button(root, text="Clik Me", command=on_button_click,bg="lavender",fg="black")
button.pack(pady=10)

browse_button = tk.Button(root, text="Browse", command=browse_file, bg= "blue", fg="white")
browse_button.pack(pady=10)

close_button = tk.Button(root, text="close", command = root.quit,bg = "red", fg= "white")
close_button.pack(pady=10)



root.mainloop()

//tk
import tkinter as tk
from tkinter import filedialog

def on_button_click():
    label.config(text="Button Clicked!")

def browse_file():
    file_path = filedialog.askopenfilename(title="Select a File", filetypes=(("Text files", "*.txt"),("All files","*.*")))
    if file_path:
        file_label.config(text=f"Selected File: {file_path}")

root = tk.Tk()
root.title("Simple Tkinter App for Bharani")
root.geometry("500x500")
root.config(bg="lightblue")

label = tk.Label(root, text="Hello, Tkinter!", bg="lightyellow", fg="black")
label.pack(pady=10)

button = tk.Button(root, text="Click Me", command=on_button_click, bg="lavender", fg="black")
button.pack(pady=10)

# Create a frame to hold the browse button and the file path label
browse_frame = tk.Frame(root, bg="lightblue")
browse_frame.pack(pady=10)

browse_button = tk.Button(browse_frame, text="Browse", command=browse_file, bg="blue", fg="white")
browse_button.pack(side="left", padx=5)

file_label = tk.Label(browse_frame, text="", bg="lightblue", fg="black")
file_label.pack(side="left", padx=5)

close_button = tk.Button(root, text="Close", command=root.quit, bg="red", fg="white")
close_button.pack(pady=10)

root.mainloop()
//update
import tkinter as tk
from tkinter import filedialog, ttk
from tkinter import scrolledtext

class FileComparerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("File Comparer by Bharani's App")
        
        
        # Create the Notebook (tab container)
        self.notebook = ttk.Notebook(root)
        self.notebook.pack(expand=1, fill='both')

        style = ttk.Style()
        style.configure('TFrame', background='lavender')
        style.configure('TButton', background='lightgreen')
        style.configure('TLabel', background='lightgray')


        # Tab 1: File Comparison
        self.tab1 = ttk.Frame(self.notebook)
        self.notebook.add(self.tab1, text='File Comparison')

        # Tab 2: Placeholder
        self.tab2 = ttk.Frame(self.notebook)
        self.notebook.add(self.tab2, text='Tab 2')

        # Add widgets to Tab 1
        self.create_tab1_widgets()

        # Add widgets to Tab 2 (currently empty)
        self.create_tab2_widgets()

        ##self.config(bg="lightblue")

    def create_tab1_widgets(self):
        # File path labels and browse buttons
        self.file1_path = tk.StringVar()
        self.file2_path = tk.StringVar()

        ttk.Label(self.tab1, text="File 1:").grid(row=0, column=0, padx=10, pady=10)
        ttk.Entry(self.tab1, textvariable=self.file1_path, width=50).grid(row=0, column=1, padx=10, pady=10)
        ttk.Button(self.tab1, text="Browse", command=self.browse_file1).grid(row=0, column=2, padx=10, pady=10)

        ttk.Label(self.tab1, text="File 2:").grid(row=1, column=0, padx=10, pady=10)
        ttk.Entry(self.tab1, textvariable=self.file2_path, width=50).grid(row=1, column=1, padx=10, pady=10)
        ttk.Button(self.tab1, text="Browse", command=self.browse_file2).grid(row=1, column=2, padx=10, pady=10)

        # Compare button
        ttk.Button(self.tab1, text="Compare", command=self.compare_files).grid(row=2, column=0, columnspan=3, pady=10)

        # Log box with scrollbars
        self.log_box = scrolledtext.ScrolledText(self.tab1, width=80, height=20)
        self.log_box.grid(row=3, column=0, columnspan=3, padx=10, pady=10)

        # Clear button
        ttk.Button(self.tab1, text="Clear", command=self.clear_log).grid(row=4, column=0, columnspan=3, pady=10)

    def create_tab2_widgets(self):
        # Placeholder for future functionality
        ttk.Label(self.tab2, text="This is Tab 2.").pack(padx=10, pady=10)

    def browse_file1(self):
        filename = filedialog.askopenfilename()
        if filename:
            self.file1_path.set(filename)

    def browse_file2(self):
        filename = filedialog.askopenfilename()
        if filename:
            self.file2_path.set(filename)

    def compare_files(self):
        file1 = self.file1_path.get()
        file2 = self.file2_path.get()

        if not file1 or not file2:
            self.log("Both files must be selected.")
            return

        try:
            with open(file1, 'r') as f1, open(file2, 'r') as f2:
                f1_content = f1.read()
                f2_content = f2.read()
                
                if f1_content == f2_content:
                    self.log("Files are identical.")
                else:
                    self.log("Files are different.")
        except Exception as e:
            self.log(f"Error comparing files: {e}")

    def log(self, message):
        self.log_box.insert(tk.END, message + "\n")
        self.log_box.yview(tk.END)

    def clear_log(self):
        self.log_box.delete(1.0, tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = FileComparerApp(root)
    root.mainloop()
//update
import tkinter as tk
from tkinter import filedialog
from tkinter import scrolledtext

def on_button_click():
    label.config(text="Button Clicked!")

def browse_file():
    file_path = filedialog.askopenfilename(title="Select a File", filetypes=(("Text files", "*.txt"),("All files","*.*")))
    if file_path:
        # Add file path to the log box
        log_box.insert(tk.END, file_path + '\n')
        log_box.yview(tk.END)  # Auto-scroll to the end of the log box

root = tk.Tk()
root.title("Simple Tkinter App for Bharani")
root.geometry("500x500")
root.config(bg="lightblue")

label = tk.Label(root, text="Hello, Tkinter!", bg="lightyellow", fg="black")
label.pack(pady=10)

button = tk.Button(root, text="Click Me", command=on_button_click, bg="lavender", fg="black")
button.pack(pady=10)

browse_button = tk.Button(root, text="Browse", command=browse_file, bg="blue", fg="white")
browse_button.pack(side="top", padx=5)

close_button = tk.Button(root, text="Close", command=root.quit, bg="red", fg="white")
close_button.pack(pady=10)

# Create the log box
log_box = scrolledtext.ScrolledText(root, wrap=tk.WORD, height=10, width=50, bg="white", fg="black")
log_box.pack(pady=10)

root.mainloop()

//update

import tkinter as tk
from tkinter import filedialog, ttk, scrolledtext, messagebox

class FileComparerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("File Comparer by Bharani's App")

        # Create the Notebook (tab container)
        self.notebook = ttk.Notebook(root)
        self.notebook.pack(expand=1, fill='both')

        style = ttk.Style()
        style.configure('TFrame', background='lavender')
        style.configure('TButton', background='lightgreen')
        style.configure('TLabel', background='lightgray')

        # Tab 1: File Comparison
        self.tab1 = ttk.Frame(self.notebook)
        self.notebook.add(self.tab1, text='File Comparison')

        # Tab 2: Placeholder
        self.tab2 = ttk.Frame(self.notebook)
        self.notebook.add(self.tab2, text='Tab 2')

        # Add widgets to Tab 1
        self.create_tab1_widgets()

        # Add widgets to Tab 2 (currently empty)
        self.create_tab2_widgets()

    def create_tab1_widgets(self):
        # File path labels and browse buttons
        self.file1_path = tk.StringVar()
        self.file2_path = tk.StringVar()

        ttk.Label(self.tab1, text="File 1:").grid(row=0, column=0, padx=10, pady=10)
        ttk.Entry(self.tab1, textvariable=self.file1_path, width=50).grid(row=0, column=1, padx=10, pady=10)
        ttk.Button(self.tab1, text="Browse", command=self.browse_file1).grid(row=0, column=2, padx=10, pady=10)

        ttk.Label(self.tab1, text="File 2:").grid(row=1, column=0, padx=10, pady=10)
        ttk.Entry(self.tab1, textvariable=self.file2_path, width=50).grid(row=1, column=1, padx=10, pady=10)
        ttk.Button(self.tab1, text="Browse", command=self.browse_file2).grid(row=1, column=2, padx=10, pady=10)

        # Compare button
        ttk.Button(self.tab1, text="Compare", command=self.compare_files).grid(row=2, column=0, columnspan=3, pady=10)

        # Log box with scrollbars
        self.log_box = scrolledtext.ScrolledText(self.tab1, width=80, height=20)
        self.log_box.grid(row=3, column=0, columnspan=3, padx=10, pady=10)

        # Clear button
        ttk.Button(self.tab1, text="Clear", command=self.clear_log).grid(row=4, column=0, columnspan=3, pady=10)

    def create_tab2_widgets(self):
        # Placeholder for future functionality
        ttk.Label(self.tab2, text="This is Tab 2.").pack(padx=10, pady=10)

    def browse_file_or_folder(self, target_var):
        # Create a dialog window to select file or folder
        dialog = tk.Toplevel(self.root)
        dialog.title("Select File or Folder")
        dialog.geometry("300x100")
        
        ttk.Label(dialog, text="Do you want to select a file or a folder?").pack(pady=10)
        
        ttk.Button(dialog, text="Select File", command=lambda: self.select_file(dialog, target_var)).pack(side=tk.LEFT, padx=20)
        ttk.Button(dialog, text="Select Folder", command=lambda: self.select_folder(dialog, target_var)).pack(side=tk.RIGHT, padx=20)
        
    def select_file(self, dialog, target_var):
        filename = filedialog.askopenfilename()
        if filename:
            target_var.set(filename)
        dialog.destroy()
    
    def select_folder(self, dialog, target_var):
        foldername = filedialog.askdirectory()
        if foldername:
            target_var.set(foldername)
        dialog.destroy()

    def browse_file1(self):
        self.browse_file_or_folder(self.file1_path)

    def browse_file2(self):
        self.browse_file_or_folder(self.file2_path)

    def compare_files(self):
        file1 = self.file1_path.get()
        file2 = self.file2_path.get()

        if not file1 or not file2:
            self.log("Both files or folders must be selected.")
            return

        try:
            with open(file1, 'r') as f1, open(file2, 'r') as f2:
                f1_content = f1.read()
                f2_content = f2.read()

                if f1_content == f2_content:
                    self.log("Files are identical.")
                else:
                    self.log("Files are different.")
        except Exception as e:
            self.log(f"Error comparing files: {e}")

    def log(self, message):
        self.log_box.insert(tk.END, message + "\n")
        self.log_box.yview(tk.END)

    def clear_log(self):
        self.log_box.delete(1.0, tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = FileComparerApp(root)
    root.mainloop()

//2

import tkinter as tk
from tkinter import filedialog, simpledialog
from tkinter import scrolledtext

def on_button_click():
    label.config(text="Button Clicked!")

def browse_file_or_folder():
    # Ask the user if they want to browse for a file or folder
    choice = simpledialog.askstring("Input", "Type 'file' to browse files or 'folder' to browse folders:")

    if choice and choice.lower() == 'file':
        file_path = filedialog.askopenfilename(title="Select a File", filetypes=(("Text files", "*.txt"),("All files","*.*")))
        if file_path:
            # Add file path to the log box with "File:" prefix
            log_box.insert(tk.END, "File: " + file_path + '\n')
            log_box.yview(tk.END)  # Auto-scroll to the end of the log box

    elif choice and choice.lower() == 'folder':
        folder_path = filedialog.askdirectory(title="Select a Folder")
        if folder_path:
            # Add folder path to the log box with "Folder:" prefix
            log_box.insert(tk.END, "Folder: " + folder_path + '\n')
            log_box.yview(tk.END)  # Auto-scroll to the end of the log box
    else:
        tk.messagebox.showwarning("Invalid Input", "Please type 'file' or 'folder'.")

def clear_log():
    log_box.delete(1.0, tk.END)  # Clear the log box

root = tk.Tk()
root.title("Simple Tkinter App for Bharani")
root.geometry("500x550")
root.config(bg="lightblue")

label = tk.Label(root, text="Hello, Tkinter!", bg="lightyellow", fg="black")
label.pack(pady=10)

button = tk.Button(root, text="Click Me", command=on_button_click, bg="lavender", fg="black")
button.pack(pady=10)

browse_button = tk.Button(root, text="Browse File/Folder", command=browse_file_or_folder, bg="blue", fg="white")
browse_button.pack(side="top", padx=5, pady=5)

clear_button = tk.Button(root, text="Clear Log", command=clear_log, bg="orange", fg="white")
clear_button.pack(pady=10)

close_button = tk.Button(root, text="Close", command=root.quit, bg="red", fg="white")
close_button.pack(pady=10)

# Create the log box
log_box = scrolledtext.ScrolledText(root, wrap=tk.WORD, height=10, width=50, bg="white", fg="black")
log_box.pack(pady=10)

root.mainloop()
//update
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
        config_file = self.config_entry.get()
        test_material_path = self.testmaterial_entry.get()
        test_vector = self.testvectors_combobox.get()
        reference_file_path = self.referane_file_entry.get()

        if not all([decoder_file, reference_file_path, config_file, test_material_path, test_vector]):
            messagebox.showerror("Error", "Please ensure all fields are filled in correctly.")
            return

        try:
            # The provided `cline` example
            cline = f"/home/bharani.vidyaakar/Test_Tools/scripts/fxp_decoder && python {decoder_file} input -i {test_material_path}/{test_vector} -r {reference_file_path} config -t {config_file}"

            # Extract portions of the command (for this example, we will just log the full command)
            extracted_portions = extract_details_from_command(cline)
            
            # Log the extracted details
            self.log_box.insert(tk.END, f"Generated Command: {cline}\n")
            for i, portion in enumerate(extracted_portions, start=1):
                self.log_box.insert(tk.END, f"Portion {i}: {portion}\n")

            # Run the process (simulating the command)
            subprocess.run(cline, check=True, shell=True)
            self.log_box.insert(tk.END, "Golden Data Generation Completed!\n")

        except subprocess.CalledProcessError as e:
            self.log_box.insert(tk.END, f"Error during golden data generation: {str(e)}\n")

    def build_source_code(self):
        folder_path = self.folder_path_entry.get()

        # Check if a valid folder path is selected
        if not os.path.isdir(folder_path):
            messagebox.showerror("Error", "Please select a valid source code folder")
            return

        # Check if CMakeLists.txt exists in the selected folder
        cmakelists_path = os.path.join(folder_path, "CMakeLists.txt")
        if not os.path.isfile(cmakelists_path):
            messagebox.showerror("Error", f"CMakeLists.txt not found in {folder_path}")
            return

        try:
            # Clear the log box before new output
            self.log_box.delete(1.0, tk.END)

            # Display the folder path in the log box
            self.log_box.insert(tk.END, f"Building source code in folder: {folder_path}\n")
            self.log_box.see(tk.END)

            # First CMake command: cmake -Bbuild
            self.log_box.insert(tk.END, "Running: cmake -Bbuild\n")
            self.log_box.see(tk.END)
            result = subprocess.run(["cmake", "-Bbuild"], cwd=folder_path, capture_output=True, text=True)
            self.log_box.insert(tk.END, result.stdout)
            self.log_box.insert(tk.END, result.stderr)
            self.log_box.see(tk.END)

            # Second CMake command: cmake --build build
            self.log_box.insert(tk.END, "Running: cmake --build build\n")
            self.log_box.see(tk.END)
            result = subprocess.run(["cmake", "--build", "build"], cwd=folder_path, capture_output=True, text=True)
            self.log_box.insert(tk.END, result.stdout)
            self.log_box.insert(tk.END, result.stderr)
            self.log_box.see(tk.END)

        except Exception as e:
            messagebox.showerror("Error", f"Error while building source code: {e}")

# Utility function for command line extraction (can be adapted further)
def extract_details_from_command(cline):
    # Example extraction (You can adapt this regex to extract other needed portions)
    regex = r'-i\s+(\S+)|-r\s+(\S+)|--config\s+(\S+)'
    matches = re.findall(regex, cline)

    extracted = [match for match_tuple in matches for match in match_tuple if match]
    return extracted

# Application window setup
if __name__ == "__main__":
    root = tk.Tk()
    root.title("Golden Data Generator")
    root.geometry("800x600")

    app = WidgetFrame(root)
    app.pack(fill="both", expand=True)

    root.mainloop()
