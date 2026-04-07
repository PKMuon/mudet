#!/usr/bin/env python3
import os
import subprocess
import argparse
from multiprocessing import Pool, cpu_count

# configuration
ELEMENT = "Th"
Z = 90
ISOTOPE = 232
MAX_N = 5
OUTPUT_DIR = "./.."
MUDIRAC_EXEC = "mudirac" 
SHELL_MAP = {
    1: 'K', 2: 'L', 3: 'M', 4: 'N', 5: 'O', 
    6: 'P', 7: 'Q', 8: 'R', 9: 'S', 10: 'T',
    11: 'U', 12: 'V', 13: 'W', 14: 'X'
}

# helpers
def get_quantum_numbers(k):
    if k == 1:
        return 0, 0.5
    elif k % 2 == 0:
        l = k // 2
        return l, l - 0.5
    else:
        l = (k - 1) // 2
        return l, l + 0.5

def get_transitions(n_initial, n_final):
    si = SHELL_MAP.get(n_initial)
    sf = SHELL_MAP.get(n_final)
    if not si or not sf: return []
    lines = []
    max_k_initial = 2 * n_initial - 1
    max_k_final = 2 * n_final - 1
    for ki in range(1, max_k_initial + 1):
        li, ji = get_quantum_numbers(ki)
        for kf in range(1, max_k_final + 1):
            lf, jf = get_quantum_numbers(kf)
            delta_l = abs(li - lf)
            if delta_l != 1: continue
            delta_j = abs(ji - jf)
            if delta_j > 1.001: continue
            lines.append(f"{sf}{kf}-{si}{ki}")
    return lines

def run_single(args_tuple):
    ni, nf, ELEMENT, Z, ISOTOPE = args_tuple
    lines = get_transitions(ni, nf)
    if not lines:
        print(f"Skipping n={ni} -> n={nf} (No lines defined)")
        return
    lines_str = ",".join(lines)
    job_name = f"{ELEMENT}{ISOTOPE}_N{ni}_to_N{nf}"
    in_file = f"{job_name}.in"
    with open(in_file, "w") as f:
        f.write(f"element: {ELEMENT}\n")
        f.write(f"isotope: {ISOTOPE}\n")
        f.write(f"xr_lines: {lines_str}\n")
        f.write(f"write_spec: T\n") 
        f.write(f"nuclear_model: FERMI2\n")
        f.write(f"electronic_config: {ELEMENT}\n")
        f.write(f"uehling_correction: T\n")
    print(f"Running {job_name} [Lines: {lines_str}] ...")
    try:
        subprocess.run([MUDIRAC_EXEC, in_file], check=True, stdout=subprocess.DEVNULL)
        generated_spec = f"{job_name}.spec.dat"
        if os.path.exists(generated_spec):
            target_name = f"Z{Z}_A{ISOTOPE}_Ni{ni}_Nf{nf}.dat"
            target_path = os.path.join(OUTPUT_DIR, target_name)
            os.rename(generated_spec, target_path)
            print(f" -> Saved to {target_path}")
        else:
            print(f" -> Error: Output file {generated_spec} not found.")
    except Exception as e:
        print(f" -> Failed: {e}")

# main
def run(args):
    ELEMENT = args.element
    Z = args.atomic_number
    ISOTOPE = args.isotope
    MAX_N = args.max_n
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    job_list = [(ni, nf, ELEMENT, Z, ISOTOPE) for ni in range(2, MAX_N + 1) for nf in range(1, ni)]
    with Pool(cpu_count()) as pool:
        pool.map(run_single, job_list)

# args
def main():
    parser = argparse.ArgumentParser(description="Generate MuDirac input files and run simulations.")
    parser.add_argument("-e", "--element", type=str,  help="Element symbol (example: Th)", required=True)
    parser.add_argument("-z", "--atomic_number", type=int, help="Atomic number ", required=True)
    parser.add_argument("-a", "--isotope", type=int, help="Isotope mass number ", required=True)
    parser.add_argument("-n", "--max_n", type=int, help="Maximum principal quantum number", required=True)
    args = parser.parse_args()
    run(args)

if __name__ == "__main__":
    main()
