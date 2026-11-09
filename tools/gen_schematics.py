import schemdraw, schemdraw.elements as elm
schemdraw.use('matplotlib')
schemdraw.config(fontsize=11)

def note(d, x, y, text, size=10, loc='left'):
    d += elm.Label().at((x, y)).label(text, loc=loc, fontsize=size, halign='left' if loc=='left' else 'center')

# ---------------- HUB ----------------
with schemdraw.Drawing(file='hub_schematic.pdf', show=False) as d:
    esp = elm.Ic(pins=[
        elm.IcPin(name='GPIO13', side='right', anchorname='IO13'),
        elm.IcPin(name='GPIO14', side='right', anchorname='IO14'),
        elm.IcPin(name='GPIO27', side='right', anchorname='IO27'),
        elm.IcPin(name='GPIO26', side='right', anchorname='IO26'),
        elm.IcPin(name='GPIO25', side='right', anchorname='IO25'),
        elm.IcPin(name='GPIO22', side='left', anchorname='IO22'),
        elm.IcPin(name='GPIO21', side='left', anchorname='IO21'),
        elm.IcPin(name='GPIO33', side='left', anchorname='IO33'),
        elm.IcPin(name='GPIO32', side='left', anchorname='IO32'),
        elm.IcPin(name='GPIO4', side='left', anchorname='IO4'),
        elm.IcPin(name='GPIO16', side='left', anchorname='IO16'),
        elm.IcPin(name='GPIO17', side='left', anchorname='IO17'),
        elm.IcPin(name='GPIO23', side='left', anchorname='IO23'),
        elm.IcPin(name='GPIO18', side='left', anchorname='IO18'),
        elm.IcPin(name='GPIO19', side='left', anchorname='IO19'),
        elm.IcPin(name='VIN', side='top', anchorname='VIN'),
        elm.IcPin(name='3V3', side='top', anchorname='V33'),
        elm.IcPin(name='GND', side='bottom', anchorname='GND'),
    ], pinspacing=1.8, edgepadW=3.2, edgepadH=1.2, leadlen=0.8,
       label='', lblsize=12)
    d += esp
    d += elm.Label().at((esp.IO25[0]-0.8-1.9, (esp.IO33[1]+esp.IO21[1])/2)).label('ESP32 DevKit V1\nHUB (Board 1)\nUSB 5 V power', fontsize=12)
    d += elm.Line().up(0.6).at(esp.VIN); d += elm.Vdd().label('5V (VIN)')
    d += elm.Line().up(0.6).at(esp.V33); d += elm.Vdd().label('3V3')
    d += elm.Ground().at(esp.GND)

    # Siren driver on GPIO25
    d += elm.Resistor().right().at(esp.IO25).label('1 kΩ').length(2.5)
    q = elm.BjtNpn(circle=True).anchor('base').label('Q1\nBC547', loc='right')
    d += q
    d += elm.Ground().at(q.emitter)
    d += elm.Line().up(0.5).at(q.collector)
    d += elm.RBox().up().label('BZ1 active\nbuzzer (- at C)', loc='bottom').length(2.2)
    d += elm.Line().up(0.4); d += elm.Vdd().label('5V (VIN)')

    # LEDs
    for pin, nm in [('IO26','READY'),('IO27','ARMED'),('IO14','ALARM'),('IO13','FAULT')]:
        d += elm.Line().right(4.5).at(getattr(esp, pin))
        d += elm.Resistor().right().label('330 Ω').length(2.2)
        d += elm.LED().right().label(nm, loc='bottom').length(2)
        d += elm.Ground().right()

    # Keypad block
    kp = elm.Ic(pins=[elm.IcPin(name=n, side='right', anchorname=n) for n in
                      ['C4','C3','C2','C1','R4','R3','R2','R1']],
                pinspacing=1.8, edgepadW=1.2, leadlen=0.6,
                label='4x4 keypad\n(pins 1-8)', lblsize=12)
    d += kp.anchor('R1').at((esp.IO19[0]-6, esp.IO19[1]))
    for kpin, ep in zip(['R1','R2','R3','R4','C1','C2','C3','C4'],
                        ['IO19','IO18','IO23','IO17','IO16','IO4','IO32','IO33']):
        d += elm.Line().at(getattr(kp, kpin)).to(getattr(esp, ep))

    # OLED block
    ol = elm.Ic(pins=[elm.IcPin(name=n, side='right', anchorname=n) for n in ['GND','VCC','SCL','SDA']],
                pinspacing=1.8, edgepadW=1.2, leadlen=0.6,
                label='SSD1306\nOLED 128x64\nI2C 0x3C', lblsize=12)
    d += ol.anchor('SDA').at((esp.IO21[0]-6, esp.IO21[1]))
    d += elm.Line().at(ol.SDA).to(esp.IO21)
    d += elm.Line().at(ol.SCL).to(esp.IO22)
    d += elm.Line().right(0.8).at(ol.VCC); d += elm.Vdd().label('3V3')
    d += elm.Line().right(0.8).at(ol.GND); d += elm.Line().down(0.4); d += elm.Ground()

    bb = d.get_bbox(); bx = bb.xmin; by = bb.ymin-1.2
    note(d, bx, by, 'HomeGuard  -  HUB schematic  (25ES604 ESD mini-project)   rev 1   2026-11-05', 12)
    note(d, bx, by-0.8, 'Keypad pin 1-4 = rows R1-R4, pin 5-8 = columns C1-C4 (Keypad library pulls rows/cols internally).  All GND symbols are the common ESP32 GND.')
    note(d, bx, by-1.5, 'Siren: Ib = (3.3-0.7)/1k = 2.6 mA, BC547 saturated, Vce(sat) <= 0.2 V.  LEDs: (3.3-2.0)/330 = ~4 mA each.  Buzzer runs from 5 V VIN.')

# ---------------- DETECTOR ----------------
with schemdraw.Drawing(file='detector_schematic.pdf', show=False) as d:
    esp = elm.Ic(pins=[
        elm.IcPin(name='GPIO26', side='right', anchorname='IO26'),
        elm.IcPin(name='GPIO25', side='right', anchorname='IO25'),
        elm.IcPin(name='GPIO27', side='left', anchorname='IO27'),
        elm.IcPin(name='VIN', side='top', anchorname='VIN'),
        elm.IcPin(name='3V3', side='top', anchorname='V33'),
        elm.IcPin(name='GND', side='bottom', anchorname='GND'),
    ], pinspacing=5, edgepadW=3.6, edgepadH=1.2, leadlen=0.8,
       label='', lblsize=12)
    d += esp
    d += elm.Label().at((esp.IO25[0]-0.8-3.6, (esp.IO25[1]+esp.IO27[1])/2)).label('ESP32 DevKit V1\nDETECTOR (Board 2)\nUSB 5 V / power bank\nOnboard LED GPIO2 = TX blink', fontsize=12)
    d += elm.Line().up(0.6).at(esp.VIN); d += elm.Vdd().label('5V (VIN)')
    d += elm.Line().up(0.6).at(esp.V33); d += elm.Vdd().label('3V3')
    d += elm.Ground().at(esp.GND)

    for pin, sw, swl, rl, cl in [
        ('IO26', 'Reed MC-38\n(closed = door shut)', 'S1', 'R1 10 kΩ', 'C1 100 nF'),
        ('IO25', 'Tamper microswitch\nCOM-NO (closed = lid on)', 'S2', 'R2 10 kΩ', 'C2 100 nF')]:
        p = getattr(esp, pin)
        d += elm.Line().right(1.2).at(p)
        n1 = d.here
        d += elm.Dot()
        d += elm.Resistor().up().label(rl, loc='bottom').length(2.2)
        d += elm.Vdd().label('3V3')
        d += elm.Line().right(2.0).at(n1)
        n2 = d.here
        d += elm.Dot()
        d += elm.Capacitor().down().label(cl, loc='bottom').length(2.2)
        d += elm.Ground()
        d += elm.Line().right(1.2).at(n2)
        d += elm.Switch().right().label(swl + '  ' + sw, loc='top', fontsize=10).length(2.5)
        d += elm.Ground().right()

    pir = elm.Ic(pins=[elm.IcPin(name=n, side='right', anchorname=n) for n in ['GND','OUT','VCC']],
                 pinspacing=1.8, edgepadW=2.2, leadlen=0.6,
                 label='HC-SR501\nPIR\n(jumper H)', lblsize=12, lblofst=-0.6)
    d += pir.anchor('OUT').at((esp.IO27[0]-5, esp.IO27[1]))
    d += elm.Line().at(pir.OUT).to(esp.IO27)
    d += elm.Line().right(0.8).at(pir.VCC); d += elm.Line().up(0.4); d += elm.Vdd().label('5V (VIN)')
    d += elm.Line().right(0.8).at(pir.GND); d += elm.Line().down(0.4); d += elm.Ground()

    bb = d.get_bbox(); bx = bb.xmin; by = bb.ymin-1.2
    note(d, bx, by, 'HomeGuard  -  DETECTOR schematic  (25ES604 ESD mini-project)   rev 1   2026-11-05', 12)
    note(d, bx, by-0.8, 'Fail-safe inputs: switch to GND + pull-up, so a cut wire reads OPEN (alarm side). Internal pull-up also enabled.')
    note(d, bx, by-1.5, 'RC filter: 10 kΩ x 100 nF = 1 ms; firmware debounce 50 ms.  PIR needs 5 V supply; OUT is 3.3 V logic (safe for GPIO).')
