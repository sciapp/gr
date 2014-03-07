# -*- coding: utf-8 -*-
"""
This module includes information about elements:
- atom_name_list maps element number to name
- atom_color_list maps element number to rgb color tuple
- atomic_number_dict maps element symbol to element number (not a 1:1, but a n:1 mapping, see Ununbium (UUB)/Copernicum(CN))
- atom_radius_list maps element number to covalent radius in mÅ
"""

atom_name_list = [None,"Hydrogen","Helium","Lithium","Beryllium","Boron","Carbon","Nitrogen","Oxygen","Fluorine","Neon","Sodium","Magnesium","Aluminium","Silicon","Phosphorus","Sulfur","Chlorine","Argon","Potassium","Calcium","Scandium","Titanium","Vanadium","Chromium","Manganese","Iron","Cobalt","Nickel","Copper","Zinc","Gallium","Germanium","Arsenic","Selenium","Bromine","Krypton","Rubidium","Strontium","Yttrium","Zirconium","Niobium","Molybdenum","Technetium","Ruthenium","Rhodium","Palladium","Silver","Cadmium","Indium","Tin","Antimony","Tellurium","Iodine","Xenon","Caesium","Barium","Lanthanum","Cerium","Praseodymium","Neodymium","Promethium","Samarium","Europium","Gadolinium","Terbium","Dysprosium","Holmium","Erbium","Thulium","Ytterbium","Lutetium","Hafnium","Tantalum","Tungsten","Rhenium","Osmium","Iridium","Platinum","Gold","Mercury","Thallium","Lead","Bismuth","Polonium","Astatine","Radon","Francium","Radium","Actinium","Thorium","Protactinium","Uranium","Neptunium","Plutonium","Americium","Curium","Berkelium","Californium","Einsteinium","Fermium","Mendelevium","Nobelium","Lawrencium","Rutherfordium","Dubnium","Seaborgium","Bohrium","Hassium","Meitnerium","Darmstadtium","Roentgenium","Copernicium","Ununtrium","Ununquadium","Ununpentium","Ununhexium","Ununseptium","Ununoctium"]
atom_color_list = [None]*119
atomic_number_dict = {}
atomic_number_dict["H"] = 1
atom_color_list[ 1 ] = (255, 255, 255)
atomic_number_dict["HE"] = 2
atom_color_list[ 2 ] = (217, 255, 255)
atomic_number_dict["LI"] = 3
atom_color_list[ 3 ] = (204, 128, 255)
atomic_number_dict["BE"] = 4
atom_color_list[ 4 ] = (194, 255, 0)
atomic_number_dict["B"] = 5
atom_color_list[ 5 ] = (255, 181, 181)
atomic_number_dict["C"] = 6
atom_color_list[ 6 ] = (144, 144, 144)
atomic_number_dict["N"] = 7
atom_color_list[ 7 ] = (48, 80, 248)
atomic_number_dict["O"] = 8
atom_color_list[ 8 ] = (255, 13, 13)
atomic_number_dict["F"] = 9
atom_color_list[ 9 ] = (144, 224, 80)
atomic_number_dict["NE"] = 10
atom_color_list[ 10 ] = (179, 227, 245)
atomic_number_dict["NA"] = 11
atom_color_list[ 11 ] = (171, 92, 242)
atomic_number_dict["MG"] = 12
atom_color_list[ 12 ] = (138, 255, 0)
atomic_number_dict["AL"] = 13
atom_color_list[ 13 ] = (191, 166, 166)
atomic_number_dict["SI"] = 14
atom_color_list[ 14 ] = (240, 200, 160)
atomic_number_dict["P"] = 15
atom_color_list[ 15 ] = (255, 128, 0)
atomic_number_dict["S"] = 16
atom_color_list[ 16 ] = (255, 255, 48)
atomic_number_dict["CL"] = 17
atom_color_list[ 17 ] = (31, 240, 31)
atomic_number_dict["AR"] = 18
atom_color_list[ 18 ] = (128, 209, 227)
atomic_number_dict["K"] = 19
atom_color_list[ 19 ] = (143, 64, 212)
atomic_number_dict["CA"] = 20
atom_color_list[ 20 ] = (61, 225, 0)
atomic_number_dict["SC"] = 21
atom_color_list[ 21 ] = (230, 230, 230)
atomic_number_dict["TI"] = 22
atom_color_list[ 22 ] = (191, 194, 199)
atomic_number_dict["V"] = 23
atom_color_list[ 23 ] = (166, 166, 171)
atomic_number_dict["CR"] = 24
atom_color_list[ 24 ] = (138, 153, 199)
atomic_number_dict["MN"] = 25
atom_color_list[ 25 ] = (156, 122, 199)
atomic_number_dict["FE"] = 26
atom_color_list[ 26 ] = (224, 102, 51)
atomic_number_dict["CO"] = 27
atom_color_list[ 27 ] = (240, 144, 160)
atomic_number_dict["NI"] = 28
atom_color_list[ 28 ] = (80, 208, 80)
atomic_number_dict["CU"] = 29
atom_color_list[ 29 ] = (200, 128, 51)
atomic_number_dict["ZN"] = 30
atom_color_list[ 30 ] = (125, 128, 176)
atomic_number_dict["GA"] = 31
atom_color_list[ 31 ] = (194, 143, 143)
atomic_number_dict["GE"] = 32
atom_color_list[ 32 ] = (102, 143, 143)
atomic_number_dict["AS"] = 33
atom_color_list[ 33 ] = (189, 128, 227)
atomic_number_dict["SE"] = 34
atom_color_list[ 34 ] = (225, 161, 0)
atomic_number_dict["BR"] = 35
atom_color_list[ 35 ] = (166, 41, 41)
atomic_number_dict["KR"] = 36
atom_color_list[ 36 ] = (92, 184, 209)
atomic_number_dict["RB"] = 37
atom_color_list[ 37 ] = (112, 46, 176)
atomic_number_dict["SR"] = 38
atom_color_list[ 38 ] = (0, 255, 0)
atomic_number_dict["Y"] = 39
atom_color_list[ 39 ] = (148, 255, 255)
atomic_number_dict["ZR"] = 40
atom_color_list[ 40 ] = (148, 224, 224)
atomic_number_dict["NB"] = 41
atom_color_list[ 41 ] = (115, 194, 201)
atomic_number_dict["MO"] = 42
atom_color_list[ 42 ] = (84, 181, 181)
atomic_number_dict["TC"] = 43
atom_color_list[ 43 ] = (59, 158, 158)
atomic_number_dict["RU"] = 44
atom_color_list[ 44 ] = (36, 143, 143)
atomic_number_dict["RH"] = 45
atom_color_list[ 45 ] = (10, 125, 140)
atomic_number_dict["PD"] = 46
atom_color_list[ 46 ] = (0, 105, 133)
atomic_number_dict["AG"] = 47
atom_color_list[ 47 ] = (192, 192, 192)
atomic_number_dict["CD"] = 48
atom_color_list[ 48 ] = (255, 217, 143)
atomic_number_dict["IN"] = 49
atom_color_list[ 49 ] = (166, 117, 115)
atomic_number_dict["SN"] = 50
atom_color_list[ 50 ] = (102, 128, 128)
atomic_number_dict["SB"] = 51
atom_color_list[ 51 ] = (158, 99, 181)
atomic_number_dict["TE"] = 52
atom_color_list[ 52 ] = (212, 122, 0)
atomic_number_dict["I"] = 53
atom_color_list[ 53 ] = (148, 0, 148)
atomic_number_dict["XE"] = 54
atom_color_list[ 54 ] = (66, 158, 176)
atomic_number_dict["CS"] = 55
atom_color_list[ 55 ] = (87, 23, 143)
atomic_number_dict["BA"] = 56
atom_color_list[ 56 ] = (0, 201, 0)
atomic_number_dict["LA"] = 57
atom_color_list[ 57 ] = (112, 212, 255)
atomic_number_dict["CE"] = 58
atom_color_list[ 58 ] = (255, 255, 199)
atomic_number_dict["PR"] = 59
atom_color_list[ 59 ] = (217, 225, 199)
atomic_number_dict["ND"] = 60
atom_color_list[ 60 ] = (199, 225, 199)
atomic_number_dict["PM"] = 61
atom_color_list[ 61 ] = (163, 225, 199)
atomic_number_dict["SM"] = 62
atom_color_list[ 62 ] = (143, 225, 199)
atomic_number_dict["EU"] = 63
atom_color_list[ 63 ] = (97, 225, 199)
atomic_number_dict["GD"] = 64
atom_color_list[ 64 ] = (69, 225, 199)
atomic_number_dict["TB"] = 65
atom_color_list[ 65 ] = (48, 225, 199)
atomic_number_dict["DY"] = 66
atom_color_list[ 66 ] = (31, 225, 199)
atomic_number_dict["HO"] = 67
atom_color_list[ 67 ] = (0, 225, 156)
atomic_number_dict["ER"] = 68
atom_color_list[ 68 ] = (0, 230, 117)
atomic_number_dict["TM"] = 69
atom_color_list[ 69 ] = (0, 212, 82)
atomic_number_dict["YB"] = 70
atom_color_list[ 70 ] = (0, 191, 56)
atomic_number_dict["LU"] = 71
atom_color_list[ 71 ] = (0, 171, 36)
atomic_number_dict["HF"] = 72
atom_color_list[ 72 ] = (77, 194, 255)
atomic_number_dict["TA"] = 73
atom_color_list[ 73 ] = (77, 166, 255)
atomic_number_dict["W"] = 74
atom_color_list[ 74 ] = (33, 148, 214)
atomic_number_dict["RE"] = 75
atom_color_list[ 75 ] = (38, 125, 171)
atomic_number_dict["OS"] = 76
atom_color_list[ 76 ] = (38, 102, 150)
atomic_number_dict["IR"] = 77
atom_color_list[ 77 ] = (23, 84, 135)
atomic_number_dict["PT"] = 78
atom_color_list[ 78 ] = (208, 208, 224)
atomic_number_dict["AU"] = 79
atom_color_list[ 79 ] = (255, 209, 35)
atomic_number_dict["HG"] = 80
atom_color_list[ 80 ] = (184, 184, 208)
atomic_number_dict["TL"] = 81
atom_color_list[ 81 ] = (166, 84, 77)
atomic_number_dict["PB"] = 82
atom_color_list[ 82 ] = (87, 89, 97)
atomic_number_dict["BI"] = 83
atom_color_list[ 83 ] = (158, 79, 181)
atomic_number_dict["PO"] = 84
atom_color_list[ 84 ] = (171, 92, 0)
atomic_number_dict["AT"] = 85
atom_color_list[ 85 ] = (117, 79, 69)
atomic_number_dict["RN"] = 86
atom_color_list[ 86 ] = (66, 130, 150)
atomic_number_dict["FR"] = 87
atom_color_list[ 87 ] = (66, 0, 102)
atomic_number_dict["RA"] = 88
atom_color_list[ 88 ] = (0, 125, 0)
atomic_number_dict["AC"] = 89
atom_color_list[ 89 ] = (112, 171, 250)
atomic_number_dict["TH"] = 90
atom_color_list[ 90 ] = (0, 186, 255)
atomic_number_dict["PA"] = 91
atom_color_list[ 91 ] = (0, 161, 255)
atomic_number_dict["U"] = 92
atom_color_list[ 92 ] = (0, 143, 255)
atomic_number_dict["NP"] = 93
atom_color_list[ 93 ] = (0, 128, 255)
atomic_number_dict["PU"] = 94
atom_color_list[ 94 ] = (0, 107, 255)
atomic_number_dict["AM"] = 95
atom_color_list[ 95 ] = (84, 92, 242)
atomic_number_dict["CM"] = 96
atom_color_list[ 96 ] = (120, 92, 227)
atomic_number_dict["BK"] = 97
atom_color_list[ 97 ] = (138, 79, 227)
atomic_number_dict["CF"] = 98
atom_color_list[ 98 ] = (161, 54, 212)
atomic_number_dict["ES"] = 99
atom_color_list[ 99 ] = (179, 31, 212)
atomic_number_dict["FM"] = 100
atom_color_list[ 100 ] = (179, 31, 186)
atomic_number_dict["MD"] = 101
atom_color_list[ 101 ] = (179, 13, 166)
atomic_number_dict["NO"] = 102
atom_color_list[ 102 ] = (189, 13, 135)
atomic_number_dict["LR"] = 103
atom_color_list[ 103 ] = (199, 0, 102)
atomic_number_dict["RF"] = 104
atom_color_list[ 104 ] = (204, 0, 89)
atomic_number_dict["DB"] = 105
atom_color_list[ 105 ] = (209, 0, 79)
atomic_number_dict["SG"] = 106
atom_color_list[ 106 ] = (217, 0, 69)
atomic_number_dict["BH"] = 107
atom_color_list[ 107 ] = (224, 0, 56)
atomic_number_dict["HS"] = 108
atom_color_list[ 108 ] = (230, 0, 46)
atomic_number_dict["MT"] = 109
atom_color_list[ 109 ] = (235, 0, 38)
atomic_number_dict["DS"] = 110
atom_color_list[ 110 ] = (255, 0, 255)
atomic_number_dict["RG"] = 111
atom_color_list[ 111 ] = (255, 0, 255)
atomic_number_dict["CN"] = 112
atomic_number_dict["UUB"] = 112
atom_color_list[ 112 ] = (255, 0, 255)
atomic_number_dict["UUT"] = 113
atom_color_list[ 113 ] = (255, 0, 255)
atomic_number_dict["UUQ"] = 114
atom_color_list[ 114 ] = (255, 0, 255)
atomic_number_dict["UUP"] = 115
atom_color_list[ 115 ] = (255, 0, 255)
atomic_number_dict["UUH"] = 116
atom_color_list[ 116 ] = (255, 0, 255)
atomic_number_dict["UUS"] = 117
atom_color_list[ 117 ] = (255, 0, 255)
atomic_number_dict["UUO"] = 118
atom_color_list[ 118 ] = (255, 0, 255)

# Source: Jmol constants, units: mÅ
atom_radius_list = [ None,
230,  #   1  H
930,  #   2  He
680,  #   3  Li
350,  #   4  Be
830,  #   5  B
680,  #   6  C
680,  #   7  N
680,  #   8  O
640,  #   9  F
1120, #  10  Ne
970,  #  11  Na
1100, #  12  Mg
1350, #  13  Al
1200, #  14  Si
750,  #  15  P
1020, #  16  S
990,  #  17  Cl
1570, #  18  Ar
1330, #  19  K
990,  #  20  Ca
1440, #  21  Sc
1470, #  22  Ti
1330, #  23  V
1350, #  24  Cr
1350, #  25  Mn
1340, #  26  Fe
1330, #  27  Co
1500, #  28  Ni
1520, #  29  Cu
1450, #  30  Zn
1220, #  31  Ga
1170, #  32  Ge
1210, #  33  As
1220, #  34  Se
1210, #  35  Br
1910, #  36  Kr
1470, #  37  Rb
1120, #  38  Sr
1780, #  39  Y
1560, #  40  Zr
1480, #  41  Nb
1470, #  42  Mo
1350, #  43  Tc
1400, #  44  Ru
1450, #  45  Rh
1500, #  46  Pd
1590, #  47  Ag
1690, #  48  Cd
1630, #  49  In
1460, #  50  Sn
1460, #  51  Sb
1470, #  52  Te
1400, #  53  I
1980, #  54  Xe
1670, #  55  Cs
1340, #  56  Ba
1870, #  57  La
1830, #  58  Ce
1820, #  59  Pr
1810, #  60  Nd
1800, #  61  Pm
1800, #  62  Sm
1990, #  63  Eu
1790, #  64  Gd
1760, #  65  Tb
1750, #  66  Dy
1740, #  67  Ho
1730, #  68  Er
1720, #  69  Tm
1940, #  70  Yb
1720, #  71  Lu
1570, #  72  Hf
1430, #  73  Ta
1370, #  74  W
1350, #  75  Re
1370, #  76  Os
1320, #  77  Ir
1500, #  78  Pt
1500, #  79  Au
1700, #  80  Hg
1550, #  81  Tl
1540, #  82  Pb
1540, #  83  Bi
1680, #  84  Po
1700, #  85  At
2400, #  86  Rn
2000, #  87  Fr
1900, #  88  Ra
1880, #  89  Ac
1790, #  90  Th
1610, #  91  Pa
1580, #  92  U
1550, #  93  Np
1530, #  94  Pu
1510, #  95  Am
1500, #  96  Cm
1500, #  97  Bk
1500, #  98  Cf
1500, #  99  Es
1500, # 100  Fm
1500, # 101  Md
1500, # 102  No
1500, # 103  Lr
1600, # 104  Rf
1600, # 105  Db
1600, # 106  Sg
1600, # 107  Bh
1600, # 108  Hs
1600, # 109  Mt

1600, # 110 DS
1600, # 111 RG
1600, # 112 CN
1600, # 113 UUT
1600, # 114 UUQ
1600, # 115 UUP
1600, # 116 UUH
1600, # 117 UUS
1600, # 118 UUO

]
