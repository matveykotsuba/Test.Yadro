import csv
import os
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def read_rows(file_name):
    rows = []
    with open(file_name, newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)
        for row in reader:
            item = {
                "p": float(row["channel_probability"]),
                "decoded_ber": float(row["decoded_ber"]),
                "bits": int(row["information_bits"]),
            }
            rows.append(item)
    return rows


def get_probability(row):
    return row["p"]


def main():
    input_file = "output/viterbi_ber.csv"
    output_file = "output/viterbi_ber.png"

    if len(sys.argv) > 1:
        input_file = sys.argv[1]
    if len(sys.argv) > 2:
        output_file = sys.argv[2]

    rows = read_rows(input_file)
    rows.sort(key=get_probability)

    if len(rows) == 0:
        print("no data")
        return 1

    min_ber = 0.5 / rows[0]["bits"]

    x = []
    y = []
    channel_y = []

    for row in rows:
        p = row["p"]
        decoded_ber = row["decoded_ber"]

        if decoded_ber < min_ber:
            decoded_ber = min_ber

        x.append(p)
        y.append(decoded_ber)

        if p < min_ber:
            channel_y.append(min_ber)
        else:
            channel_y.append(p)

    plt.figure(figsize=(9, 6))
    plt.plot(x, y, marker="o", linewidth=2, label="after Viterbi decoder")
    plt.plot(x, channel_y, linestyle="--", linewidth=1.5, label="BSC probability")

    plt.xlabel("Channel bit error probability")
    plt.ylabel("Bit error probability after decoder")
    plt.title("Viterbi decoder BER for binary symmetric channel")
    plt.yscale("log")
    plt.grid(True, which="both", linestyle="--", alpha=0.5)
    plt.legend()
    plt.tight_layout()

    output_dir = os.path.dirname(output_file)
    if output_dir != "":
        os.makedirs(output_dir, exist_ok=True)

    plt.savefig(output_file, dpi=200)
    print("plot saved to " + output_file)

    return 0


if __name__ == "__main__":
    sys.exit(main())
