set -e

cargo build --target=x86_64-unknown-none

./scripts/root.sh
