# Traceability Report

This report is automatically generated with each build of the documentation,
by using the DSPC req-tracer tool.

```python exec="on" html="true"
import re  # markdown-exec: hide
import subprocess  # markdown-exec: hide

cmd = "req-tracer --config scripts/reqtracing-cfg-html.yml"
c_str = subprocess.check_output(cmd.split()).decode("utf-8")  # markdown-exec: hide

style_m = re.search(r'<style>(.*?)</style>', c_str, re.DOTALL)  # markdown-exec: hide
body_m  = re.search(r'<body>(.*?)</body>',   c_str, re.DOTALL)  # markdown-exec: hide

style = f'<style>{style_m.group(1)}</style>' if style_m else ''  # markdown-exec: hide
style  = style.replace("body { font-family: sans-serif; margin: 20px; background-color: #f8f9fa; }", "")

body  = body_m.group(1) if body_m else c_str  # markdown-exec: hide
body  = body.replace("<h1>Traceability Report</h1>", "")  # markdown-exec: hide

print(style + body)  # markdown-exec: hide
```
