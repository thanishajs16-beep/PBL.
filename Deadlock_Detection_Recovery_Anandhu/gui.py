import os
import subprocess
import tkinter as tk
from tkinter import ttk, messagebox

APP_DIR = os.path.dirname(os.path.abspath(__file__))
C_SOURCE = os.path.join(APP_DIR, "deadlock.c")
EXECUTABLE = os.path.join(APP_DIR, "deadlock.exe" if os.name == "nt" else "deadlock")

def compile_backend():
    if os.name == "nt":
        # Requires GCC from MinGW-w64 or MSYS2.
        command = ["gcc", C_SOURCE, "-o", EXECUTABLE]
    else:
        command = ["gcc", C_SOURCE, "-o", EXECUTABLE]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(result.stderr or "Compilation failed.")
    return EXECUTABLE

class DeadlockApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Deadlock Detection and Recovery — Student 3")
        self.root.geometry("980x760")
        self.root.minsize(850, 650)
        self.n_var = tk.IntVar(value=2)
        self.m_var = tk.IntVar(value=2)
        self.available_entries = []
        self.allocation_entries = []
        self.request_entries = []
        self.process_frame = None
        self.resource_frame = None
        self.matrix_frame = None
        self.backend = None

        style = ttk.Style()
        try:
            style.theme_use("clam")
        except tk.TclError:
            pass

        top = ttk.Frame(root, padding=12)
        top.pack(fill="x")
        ttk.Label(top, text="Deadlock Detection and Recovery", font=("Segoe UI", 18, "bold")).pack(anchor="w")
        ttk.Label(top, text="Student 3 module • C algorithm + Tkinter GUI", font=("Segoe UI", 10)).pack(anchor="w", pady=(2, 10))

        setup = ttk.LabelFrame(root, text="1. System setup", padding=10)
        setup.pack(fill="x", padx=12, pady=5)
        ttk.Label(setup, text="Processes (1–20):").grid(row=0, column=0, sticky="w")
        ttk.Spinbox(setup, from_=1, to=20, textvariable=self.n_var, width=6).grid(row=0, column=1, padx=6)
        ttk.Label(setup, text="Resource types (1–20):").grid(row=0, column=2, sticky="w", padx=(15, 0))
        ttk.Spinbox(setup, from_=1, to=20, textvariable=self.m_var, width=6).grid(row=0, column=3, padx=6)
        ttk.Button(setup, text="Create / Reset matrices", command=self.create_matrices).grid(row=0, column=4, padx=12)
        ttk.Button(setup, text="Load deadlock demo", command=self.load_demo).grid(row=0, column=5, padx=4)

        self.matrix_container = ttk.Frame(root, padding=(12, 4))
        self.matrix_container.pack(fill="x")
        self.create_matrices()

        actions = ttk.LabelFrame(root, text="2. Run operation", padding=10)
        actions.pack(fill="x", padx=12, pady=5)
        ttk.Button(actions, text="Detect deadlock", command=lambda: self.run_operation("detect")).pack(side="left", padx=4)
        ttk.Button(actions, text="Generate RAG data", command=lambda: self.run_operation("graph")).pack(side="left", padx=4)
        ttk.Label(actions, text="Terminate process:").pack(side="left", padx=(18, 4))
        self.selected_var = tk.IntVar(value=2)
        self.selected_box = ttk.Spinbox(actions, from_=1, to=20, textvariable=self.selected_var, width=5)
        self.selected_box.pack(side="left", padx=4)
        ttk.Button(actions, text="Recover", command=lambda: self.run_operation("recover")).pack(side="left", padx=4)
        ttk.Button(actions, text="Clear output", command=self.clear_output).pack(side="right", padx=4)

        out_frame = ttk.LabelFrame(root, text="3. Results", padding=10)
        out_frame.pack(fill="both", expand=True, padx=12, pady=(5, 12))
        self.output = tk.Text(out_frame, wrap="word", height=15, font=("Consolas", 10))
        scrollbar = ttk.Scrollbar(out_frame, orient="vertical", command=self.output.yview)
        self.output.configure(yscrollcommand=scrollbar.set)
        self.output.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        self.write("Enter Available, Allocation, and Request values. Use 'Load deadlock demo' to try a working example.")

    def create_matrices(self):
        try:
            n, m = int(self.n_var.get()), int(self.m_var.get())
            if not (1 <= n <= 20 and 1 <= m <= 20):
                raise ValueError
        except (ValueError, tk.TclError):
            messagebox.showerror("Invalid size", "Processes and resources must be between 1 and 20.")
            return

        for child in self.matrix_container.winfo_children():
            child.destroy()
        self.available_entries, self.allocation_entries, self.request_entries = [], [], []

        av_frame = ttk.LabelFrame(self.matrix_container, text="Available vector", padding=8)
        av_frame.grid(row=0, column=0, sticky="nw", padx=(0, 8), pady=4)
        ttk.Label(av_frame, text="Resource").grid(row=0, column=0, padx=4)
        for j in range(m):
            ttk.Label(av_frame, text=f"R{j+1}").grid(row=0, column=j+1, padx=4)
        ttk.Label(av_frame, text="Units").grid(row=1, column=0, padx=4)
        for j in range(m):
            e = ttk.Entry(av_frame, width=5)
            e.grid(row=1, column=j+1, padx=3, pady=3)
            e.insert(0, "0")
            self.available_entries.append(e)

        alloc_frame = ttk.LabelFrame(self.matrix_container, text="Allocation matrix", padding=8)
        alloc_frame.grid(row=1, column=0, sticky="nw", padx=(0, 8), pady=4)
        req_frame = ttk.LabelFrame(self.matrix_container, text="Request matrix", padding=8)
        req_frame.grid(row=1, column=1, sticky="nw", padx=8, pady=4)

        for frame, store, title in [(alloc_frame, self.allocation_entries, "Allocated units"),
                                    (req_frame, self.request_entries, "Requested units")]:
            ttk.Label(frame, text="Process").grid(row=0, column=0, padx=4)
            for j in range(m):
                ttk.Label(frame, text=f"R{j+1}").grid(row=0, column=j+1, padx=3)
            for i in range(n):
                ttk.Label(frame, text=f"P{i+1}").grid(row=i+1, column=0, padx=4, pady=2)
                row = []
                for j in range(m):
                    e = ttk.Entry(frame, width=5)
                    e.grid(row=i+1, column=j+1, padx=3, pady=2)
                    e.insert(0, "0")
                    row.append(e)
                store.append(row)

        self.selected_box.configure(to=n)
        if self.selected_var.get() > n:
            self.selected_var.set(n)

    def load_demo(self):
        self.n_var.set(2)
        self.m_var.set(2)
        self.create_matrices()
        values = {
            "available": [0, 0],
            "allocation": [[1, 0], [0, 1]],
            "request": [[0, 1], [1, 0]],
        }
        for e, v in zip(self.available_entries, values["available"]):
            e.delete(0, "end"); e.insert(0, str(v))
        for entries, rows in [(self.allocation_entries, values["allocation"]),
                              (self.request_entries, values["request"])]:
            for row_entries, row_values in zip(entries, rows):
                for e, v in zip(row_entries, row_values):
                    e.delete(0, "end"); e.insert(0, str(v))
        self.selected_var.set(2)
        self.write("Demo loaded: P1 holds R1 and requests R2; P2 holds R2 and requests R1.\nPress Detect deadlock.")

    def read_values(self):
        n, m = int(self.n_var.get()), int(self.m_var.get())
        def read_entry(e):
            raw = e.get().strip()
            if not raw:
                raise ValueError("Every matrix cell must contain a non-negative integer.")
            value = int(raw)
            if value < 0:
                raise ValueError("Values cannot be negative.")
            return value
        available = [read_entry(e) for e in self.available_entries]
        allocation = [[read_entry(e) for e in row] for row in self.allocation_entries]
        request = [[read_entry(e) for e in row] for row in self.request_entries]
        # Each allocated or available unit is an actual resource unit. Requests cannot be validated
        # against total resources without a separate total-resource definition.
        return n, m, available, allocation, request

    def run_operation(self, operation):
        try:
            if not os.path.exists(EXECUTABLE):
                self.backend = compile_backend()
            n, m, available, allocation, request = self.read_values()
            selected = int(self.selected_var.get()) if operation == "recover" else 0
            if operation == "recover" and not (1 <= selected <= n):
                raise ValueError("Choose a process number within the current process count.")
            chunks = [f"{n} {m}", f"{operation} {selected}",
                      " ".join(map(str, available))]
            chunks += [" ".join(map(str, row)) for row in allocation]
            chunks += [" ".join(map(str, row)) for row in request]
            result = subprocess.run([EXECUTABLE], input="\n".join(chunks) + "\n",
                                    capture_output=True, text=True, timeout=5)
            if result.returncode != 0:
                raise RuntimeError(result.stderr.strip() or "C program failed.")
            self.write(result.stdout.strip())
        except (ValueError, tk.TclError) as exc:
            messagebox.showerror("Check input", str(exc))
        except (OSError, RuntimeError, subprocess.TimeoutExpired) as exc:
            messagebox.showerror("Program error", f"{exc}\n\nCheck that GCC is installed and available in PATH.")

    def write(self, text):
        self.output.insert("end", text + "\n\n")
        self.output.see("end")

    def clear_output(self):
        self.output.delete("1.0", "end")

if __name__ == "__main__":
    root = tk.Tk()
    app = DeadlockApp(root)
    root.mainloop()
