
import tkinter as tk
from tkinter import filedialog
from tkinter import scrolledtext

def on_button_click():
    label.config(text="Button Clicked!")

def browse_file():
    file_path = filedialog.askopenfilename(title="Select a File", filetypes=(("Text files", "*.txt"),("All files","*.*")))
    if file_path:
        log_box.insert(tk.END, file_path + '\n')
        log_box.yview(tk.END)

def browse_folder():
    folder_path = filedialog.askdirectory(title="Select a Folder")
    if folder_path:
        log_box.insert(tk.END, folder_path +'\n')
        log_box.yview(tk.END)

"""def browse_file_or_folder(self, target_var):
        # Create a dialog window to select file or folder
        dialog = tk.Toplevel(self.window)
        dialog.title("Select File or Folder")
        dialog.geometry("300x100")"""

def clear_log():
    log_box.delete(1.0, tk.END)

window = tk.Tk()
window.title("Golden data")
window.geometry("500x500")
window.config(bg="lightblue")

label = tk.Label(window, text="Hello, Tkinter!", bg="lavender", fg="black")
label.pack(pady=10)

button = tk.Button(window, text="Click Me", command=on_button_click, bg="lavender", fg="black")
button.pack(pady=10)


browse_button = tk.Button(window, text="Browse 1", command=browse_file, bg="lavender", fg="black")
browse_button.pack(side="top", padx=5)

browse_button = tk.Button(window, text="Browse 2",  command=browse_folder, bg="lavender", fg="black")
browse_button.pack(side= "top", pady=10)

#Label(dialog, text="Do you want to select a file or a folder?").pack(pady=10)

#browse_button(dialog, text="Select File", command=lambda: self.select_file(dialog, target_var)).pack(side=tk.LEFT, padx=20)
#browse_button(dialog, text="Select Folder", command=lambda: self.select_folder(dialog, target_var)).pack(side=tk.RIGHT, padx=20)

clear_button = tk.Button(window, text="Clear Log", command=clear_log, bg="lavender", fg="black")
clear_button.pack(pady=10)

close_button = tk.Button(window, text="Close", command=window.quit, bg="red", fg="white")
close_button.pack(pady=10)

log_box = scrolledtext.ScrolledText(window, wrap=tk.WORD, height=10, width=50, bg="black", fg="white")
log_box.pack(pady=10)

window.mainloop()


