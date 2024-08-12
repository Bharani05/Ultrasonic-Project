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
