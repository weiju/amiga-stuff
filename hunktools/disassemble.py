import struct


ADDR_MODES = {
    '000': 'Dn', '001': 'An', '010': '(An)', '011': '(An)+', '100': '-(An)',
    '101': '(d16,An)', '110': '(d8,An,Xn)', '111': 'EXT'  # -> ADDR_MODES_EXT
}

ADDR_MODES_EXT = {
    '000': '(xxx).W', '001': '(xxx).L', '100': '#<data>',
    '010': '(d16,PC)', '011': '(d8,PC,Xn)'
}

OPCODE_CATEGORIES = {
    '0000': 'bitops_movep_imm', '0001': 'move.b', '0010': 'move.l', '0011': 'move.w',
    '0100': 'misc', '1101': 'add_addx'
}

ADD_OPMODES = {
    '000': ('b', 'ea+dn->dn'), '001': ('w', 'ea+dn->dn'),'010': ('l', 'ea+dn->dn'),
    '100': ('b', 'dn+ea->ea'), '101': ('w', 'dn+ea->ea'),'110': ('l', 'dn+ea->ea')
}


def is_move(category):
    return category in {'move.b', 'move.w', 'move.l'}


def operand(size, mode_bits, reg_bits, data, offset):
    result = ""
    added = 0
    mode = ADDR_MODES[mode_bits]
    size = size.upper()

    if mode == 'EXT':
        mode = ADDR_MODES_EXT[reg_bits]
        regnum = int(reg_bits, 2)
        if mode == '#<data>':
            data_offset = offset + 2
            if size == 'L':
                imm_value = struct.unpack('>i', data[data_offset:data_offset+4])[0]
                added += 4
            elif size == 'W':
                data[data_offset:data_offset+2]
                imm_value = struct.unpack('>h', data[data_offset:data_offset+2])[0]
                added += 2
            else:
                raise Exception('unsupported size: ', size)
            result = "#%d" % imm_value
    else:
        regnum = int(reg_bits, 2)
        result = mode.replace('n', str(regnum))
    return result, added
    

def disassemble_move(bits, data, offset):
    category = OPCODE_CATEGORIES[bits[0:4]]
    total_added = 0
    # dst: reg|mode
    dst_op, added = operand(category[-1], bits[7:10], bits[4:7], data, offset)
    total_added += added

    # src: mode|reg
    src_op,added = operand(category[-1], bits[10:13], bits[13:16], data, offset)
    total_added += added

    return ((category, src_op, dst_op), total_added)


def disassemble_add(bits, data, offset):
    total_added = 0
    reg = "D%d" % int(bits[4:7], 2)
    size, operation = ADD_OPMODES[bits[7:10]]
    ea, added = operand(size, bits[10:13], bits[13:16], data, offset)
    total_added += added

    if operation == 'ea+dn->dn':
        src = ea
        dst = reg
    elif operation == 'dn+ea->ea':
        src = reg
        dst = ea
    else:
        raise Exception('Unknown operation for add')

    return (('add.%s' % size, src, dst), total_added)


def disassemble(data, offset):
    bits = "{0:016b}".format(struct.unpack(">H", data[offset:offset+2])[0])
    # first step categorize by looking at bits 15-12
    opcode = bits[0:4]
    category = OPCODE_CATEGORIES[opcode]
    if is_move(category):
        op, added = disassemble_move(bits, data, offset)
    elif category == 'add_addx':
        if bits[7] == 1 and bits[10:12] == '11':  # addx
            raise Exception('addx not supported yet')
        else:
            op, added = disassemble_add(bits, data, offset)
    else:
        print("category: ", category)
        raise Exception('TODO')
    return (op, 2 + added)


def print_instruction(address, op):
    opcode, src, dst = op
    print("$%08x:\t%8s %8s, %8s" % (address, opcode, src, dst))
