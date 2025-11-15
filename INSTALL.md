
**INSTALL.md**
```markdown
# Installation Guide

## System Requirements
- Ubuntu 20.04 LTS or 22.04 LTS
- 8GB RAM minimum, 16GB recommended
- SSD storage for graph datasets

## Step 1: Install Dependencies

```bash
sudo apt update
sudo apt install -y build-essential cmake git python3 python3-pip \
    linux-tools-common linux-tools-generic \
    libnuma-dev numactl