"""Keep documentation in published HTML without banning non-document Markdown."""
import subprocess
import sys
from pathlib import PurePosixPath


def violations(changes):
    html = any(s != 'D' and p.startswith('docs/_pages/') and p.endswith('.html') for s, p in changes)
    errors = []
    for status, path in changes:
        p = PurePosixPath(path)
        documentation = path.startswith('docs/') or (len(p.parts) == 1 and p.name not in {'AGENTS.md', 'SKILL.md'})
        if p.suffix.lower() != '.md' or not documentation or status == 'D':
            continue
        if status == 'A':
            errors.append(f'{path}: write new documentation in docs/_pages/*.html')
        elif not html:
            errors.append(f'{path}: existing guide edits require a published HTML page update')
    return errors


if __name__ == '__main__':
    raw = subprocess.check_output(['git', 'diff', '--no-renames', '--name-status', '-z', sys.argv[1], 'HEAD']).decode().split('\0')
    changes = list(zip(raw[0:-1:2], raw[1:-1:2]))
    errors = violations(changes)
    print('\n'.join(errors) if errors else 'Documentation policy passed')
    sys.exit(bool(errors))
