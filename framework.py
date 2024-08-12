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
