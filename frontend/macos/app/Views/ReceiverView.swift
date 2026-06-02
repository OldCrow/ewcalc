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
                         in: 0.001...10000, step: 0.1, decimals: 3,
                         help: "IF noise bandwidth — wider bandwidth raises the noise floor and degrades sensitivity") { adapter.setBandwidth($0) }
                InputRow("Noise figure", unit: "dB", value: $noiseFigure,
                         in: 0...30, step: 0.5,
                         help: "System noise figure — NF = 0 dB is ideal (noiseless); each additional dB raises the sensitivity floor") { adapter.setNoiseFigure($0) }
                InputRow("Required SNR", unit: "dB", value: $requiredSnr,
                         in: -20...50, step: 0.5,
                         help: "Minimum pre-detection SNR for acceptable output — depends on modulation and required BER") { adapter.setRequiredSnr($0) }
                InputRow("IIP2", unit: "dBm", value: $iip2,
                         in: -50...100,
                         help: "Second-order input intercept point — sets the 2nd-order intermodulation floor") { adapter.setSecondOrderIp($0) }
                InputRow("IIP3", unit: "dBm", value: $iip3,
                         in: -50...100,
                         help: "Third-order input intercept point — sets the 3rd-order intermodulation floor; typically the tighter limit") { adapter.setThirdOrderIp($0) }
                LabeledContent("ADC bits") {
                    HStack(spacing: 4) {
                        TextField("", value: $adcBits, format: .number)
                            .frame(width: 100)
                            .textFieldStyle(.roundedBorder)
                            .multilineTextAlignment(.trailing)
                        Stepper("", value: $adcBits, in: 1...64)
                            .labelsHidden()
                        // Blank spacer matching the unit label width in InputRow
                        Spacer().frame(width: 38)
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
                ResultRow("Sensitivity",      cStr(adapter.output.sensitivity_str),
                          help: "Minimum detectable signal: −114 + 10·log₁₀(BW) + NF + SNR (dBm)")
                ResultRow("Cascaded NF",      cStr(adapter.output.cascaded_nf_str),
                          help: "System noise figure from the Friis formula across the stage chain — front-end stage dominates")
                ResultRow("Sys. noise temp",  cStr(adapter.output.system_noise_temp_str),
                          help: "Equivalent noise temperature: Tₑ = (NFₗᵢₙ − 1) × 290 K")
                ResultRow("SFDR (2nd order)", cStr(adapter.output.sfdr2_str),
                          help: "2nd-order spurious-free dynamic range: ⅔ × (IIP2 − sensitivity)")
                ResultRow("SFDR (3rd order)", cStr(adapter.output.sfdr3_str),
                          help: "3rd-order spurious-free dynamic range: ⅔ × (IIP3 − sensitivity) — usually the binding constraint")
                ResultRow("Digital DR",       cStr(adapter.output.digital_dr_str),
                          help: "ADC quantisation dynamic range: 6.02·N + 1.76 dB")
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
            // NF group — fixed layout, left-aligned
            HStack(spacing: 4) {
                Text("S\(index + 1)")
                    .frame(width: 26, alignment: .leading)
                    .foregroundStyle(.secondary)
                Text("NF").foregroundStyle(.secondary)
                TextField("", value: $nf, format: .number.precision(.fractionLength(1)))
                    .multilineTextAlignment(.trailing)
                    .textFieldStyle(.roundedBorder)
                    .frame(width: 68)
                Stepper("", value: $nf, in: -10...50, step: 0.5).labelsHidden()
                Text("dB").foregroundStyle(.secondary).frame(width: 22)
            }

            Spacer()

            // G group — fixed layout, right-aligned; isolated so Spacer
            // cannot affect the width of the G TextField.
            HStack(spacing: 4) {
                Text("G").foregroundStyle(.secondary)
                TextField("", value: $gain, format: .number.precision(.fractionLength(1)))
                    .multilineTextAlignment(.trailing)
                    .textFieldStyle(.roundedBorder)
                    .frame(width: 68)
                Stepper("", value: $gain, in: -60...60, step: 0.5).labelsHidden()
                Text("dB").foregroundStyle(.secondary).frame(width: 22)
                Button(action: onRemove) {
                    Image(systemName: "minus.circle")
                }
                .buttonStyle(.borderless)
                .foregroundStyle(.red)
            }
        }
        .onChange(of: nf)   { onChange(EwpStageInput(noise_figure_db: $0, gain_db: gain)) }
        .onChange(of: gain)  { onChange(EwpStageInput(noise_figure_db: nf, gain_db: $0)) }
    }
}
