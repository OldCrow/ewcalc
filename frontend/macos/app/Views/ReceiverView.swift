// ReceiverView.swift
import SwiftUI

struct ReceiverView: View {
    @ObservedObject var adapter: ReceiverAdapter

    @State private var bandwidth:   Double
    @State private var noiseFigure: Double
    @State private var requiredSnr: Double
    @State private var iip2:        Double
    @State private var iip3:        Double
    @State private var adcBits:     Int
    // Local copy of the stage array; synced to the adapter on every edit.
    @State private var stages:      [EwpStageInput]

    init(adapter: ReceiverAdapter) {
        self.adapter = adapter
        _bandwidth   = State(initialValue: adapter.defaultBandwidth)
        _noiseFigure = State(initialValue: adapter.defaultNoiseFigure)
        _requiredSnr = State(initialValue: adapter.defaultRequiredSnr)
        _iip2        = State(initialValue: adapter.defaultSecondOrder)
        _iip3        = State(initialValue: adapter.defaultThirdOrder)
        _adcBits     = State(initialValue: adapter.defaultAdcBits)
        _stages      = State(initialValue: adapter.defaultStages)
    }

    var body: some View {
        Form {
            Section("System Inputs") {
                InputRow("Bandwidth", unit: "MHz", value: $bandwidth,
                         in: 0.001...10000, step: 0.1, decimals: 3) { adapter.setBandwidth($0) }
                InputRow("Noise figure", unit: "dB", value: $noiseFigure,
                         in: 0...30, step: 0.5)   { adapter.setNoiseFigure($0) }
                InputRow("Required SNR", unit: "dB", value: $requiredSnr,
                         in: -20...50, step: 0.5) { adapter.setRequiredSnr($0) }
                InputRow("IIP2", unit: "dBm", value: $iip2,
                         in: -50...100)            { adapter.setSecondOrderIp($0) }
                InputRow("IIP3", unit: "dBm", value: $iip3,
                         in: -50...100)            { adapter.setThirdOrderIp($0) }
                LabeledContent("ADC bits") {
                    HStack(spacing: 4) {
                        TextField("", value: $adcBits, format: .number)
                            .frame(width: 60)
                            .textFieldStyle(.roundedBorder)
                            .multilineTextAlignment(.trailing)
                        Stepper("", value: $adcBits, in: 1...64)
                            .labelsHidden()
                    }
                }
                .onChange(of: adcBits) { adapter.setAdcBits($0) }
            }
            Section("Noise Chain Stages") {
                ForEach(stages.indices, id: \.self) { i in
                    StageRow(index: i, stage: stages[i]) { updated in
                        stages[i] = updated
                        adapter.setStages(stages)
                    } onRemove: {
                        guard stages.count > 1 else { return }
                        stages.remove(at: i)
                        adapter.setStages(stages)
                    }
                }
                Button("+ Add stage") {
                    stages.append(EwpStageInput(noise_figure_db: 3.0, gain_db: 0.0))
                    adapter.setStages(stages)
                }
                .buttonStyle(.borderless)
            }
            Section("Results") {
                ResultRow("Sensitivity",      cStr(adapter.output.sensitivity_str))
                ResultRow("Cascaded NF",      cStr(adapter.output.cascaded_nf_str))
                ResultRow("SFDR (2nd order)", cStr(adapter.output.sfdr2_str))
                ResultRow("SFDR (3rd order)", cStr(adapter.output.sfdr3_str))
                ResultRow("Digital DR",       cStr(adapter.output.digital_dr_str))
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Receiver")
    }
}

// ── Stage row ─────────────────────────────────────────────────────────────────

private struct StageRow: View {
    let index:    Int
    let stage:    EwpStageInput
    let onChange: (EwpStageInput) -> Void
    let onRemove: () -> Void

    @State private var nf:   Double
    @State private var gain: Double

    init(index: Int, stage: EwpStageInput,
         onChange: @escaping (EwpStageInput) -> Void,
         onRemove: @escaping () -> Void)
    {
        self.index    = index
        self.stage    = stage
        self.onChange = onChange
        self.onRemove = onRemove
        _nf   = State(initialValue: stage.noise_figure_db)
        _gain = State(initialValue: stage.gain_db)
    }

    var body: some View {
        HStack(spacing: 6) {
            Text("S\(index + 1)")
                .frame(width: 26, alignment: .leading)
                .foregroundStyle(.secondary)
            Text("NF").foregroundStyle(.secondary)
            TextField("", value: $nf, format: .number.precision(.fractionLength(1)))
                .frame(width: 68).textFieldStyle(.roundedBorder).multilineTextAlignment(.trailing)
            Stepper("", value: $nf, in: -10...50, step: 0.5).labelsHidden()
            Text("dB").foregroundStyle(.secondary).frame(width: 22)
            Spacer()
            Text("G").foregroundStyle(.secondary)
            TextField("", value: $gain, format: .number.precision(.fractionLength(1)))
                .frame(width: 68).textFieldStyle(.roundedBorder).multilineTextAlignment(.trailing)
            Stepper("", value: $gain, in: -60...60, step: 0.5).labelsHidden()
            Text("dB").foregroundStyle(.secondary).frame(width: 22)
            Button(action: onRemove) {
                Image(systemName: "minus.circle")
            }
            .buttonStyle(.borderless)
            .foregroundStyle(.red)
        }
        .onChange(of: nf)   { onChange(EwpStageInput(noise_figure_db: $0, gain_db: gain)) }
        .onChange(of: gain)  { onChange(EwpStageInput(noise_figure_db: nf, gain_db: $0)) }
    }
}
