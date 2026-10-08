import re, sys
out = ["\t.module t", "\t.area HOME (CODE)", "\t.area XSEG (XDATA)", "\t.area PSEG (XDATA)", "\t.area CODE (ABS)"]
for line in open(sys.argv[1]):
    line = line.split(';')[0].rstrip()
    if not line or line.strip().upper() == 'END': continue
    m = re.match(r'^\s*(?:(\w+):)?\s*DB\s+(.*)$', line, re.I)
    if m:
        label = (m.group(1) + ':') if m.group(1) else ''
        items = re.findall(r"'[^']*'|[^,]+", m.group(2))
        parts = []
        for it in items:
            it = it.strip()
            if it.startswith("'"): parts.append('\t.ascii "%s"' % it.strip("'"))
            else: parts.append('\t.db ' + re.sub(r'\b([0-9][0-9A-Fa-f]*)H\b', lambda k: '0x' + k.group(1), it))
        out.append(label); out.extend(parts); continue
    line = re.sub(r'\b([0-9][0-9A-Fa-f]*)H\b', lambda k: '0x' + k.group(1), line)
    if line.strip().upper().startswith('ORG'): line = '\t.org ' + line.split()[1]
    out.append(line)
open(sys.argv[2], 'w').write('\n'.join(out) + '\n')
