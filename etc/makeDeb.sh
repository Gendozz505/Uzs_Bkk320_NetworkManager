#!/bin/bash

# Script to create .deb package for uzsBkk320NetworkManager target
# Usage: ./create_deb.sh [version]

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Cleanup function
cleanup() {
    log_info "Cleaning up temporary files..."
    rm -rf "$DEB_DIR"
}

# Set up cleanup trap
trap cleanup EXIT

set -e  # Exit on any error

if [ "${1:-}" = "" ]; then
    log_error "Version is required"
    exit 1
fi

# Configuration
PACKAGE_NAME="uzs-bkk320-network-manager"
VERSION=${1}
ARCHITECTURE="armhf"
MAINTAINER="Genadi M"
DESCRIPTION="TCP/UDP network manager for BKK320 configurator"
DEPENDS="libc6, libstdc++6, libgcc-s1"

# Directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
TARGET_BUILD_DIR="$BUILD_DIR/target"
DEB_DIR="$PROJECT_ROOT/deb"
DEBIAN_DIR="$DEB_DIR/DEBIAN"
INSTALL_DIR="$DEB_DIR/opt/Sensor-M/Bkk320"

# Check if target binary exists
check_target_binary() {
    if [ ! -f "$TARGET_BUILD_DIR/uzsBkk320NetworkManager" ]; then
        log_error "Target binary not found at $TARGET_BUILD_DIR/uzsBkk320NetworkManager"
        log_info "Please run 'make target' first to build the target binary"
        exit 1
    fi
    log_info "Target binary found"
}

# Build target if needed
build_target() {
    log_info "Building target binary..."
    cd "$PROJECT_ROOT"
    make target -j 8
    if [ $? -ne 0 ]; then
        log_error "Failed to build target binary"
        exit 1
    fi
    log_info "Target binary built successfully"
}

# Create package directory structure
create_package_structure() {
    log_info "Creating package directory structure..."
    
    # Create main directories
    mkdir -p "$DEBIAN_DIR"
    mkdir -p "$INSTALL_DIR/bin"
    mkdir -p "$INSTALL_DIR/data"
    mkdir -p "$INSTALL_DIR/etc/networkManager"
    mkdir -p "$DEB_DIR/etc/systemd/system"
    mkdir -p "$PROJECT_ROOT/debs"
}

# Copy files to package
copy_files() {
    log_info "Copying files to package..."
    
    # Copy binary
    cp "$TARGET_BUILD_DIR/uzsBkk320NetworkManager" "$INSTALL_DIR/bin/"
    chmod +x "$INSTALL_DIR/bin/uzsBkk320NetworkManager"
}

# Create systemd service file
create_systemd_service() {
    log_info "Creating systemd service file..."
    
    cat > "$DEB_DIR/etc/systemd/system/uzs-bkk320-network-manager.service" << EOF
[Unit]
Description=Uzs Bkk320 Network Manager
After=network.target
Wants=network.target

[Service]
ExecStart=/opt/Sensor-M/Bkk320/bin/uzsBkk320NetworkManager
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal
SyslogIdentifier=uzs-bkk320-network-manager

[Install]
WantedBy=multi-user.target
EOF
}

# Create control file
create_control_file() {
    log_info "Creating control file..."
    
    # Calculate installed size
    INSTALLED_SIZE=$(du -sk "$DEB_DIR" | cut -f1)
    
    cat > "$DEBIAN_DIR/control" << EOF
Package: $PACKAGE_NAME
Version: $VERSION
Section: net
Priority: optional
Architecture: $ARCHITECTURE
Depends: $DEPENDS
Maintainer: $MAINTAINER
Description: $DESCRIPTION
 This package provides a robust network management solution for BKK320 devices,
 featuring dual protocol support (TCP and UDP), custom message protocol with
 CRC16 validation, async I/O built on Boost.Asio, and signal-slot architecture
 using Boost.Signals2.
 .
 The application includes:
  * TCP and UDP communication handlers
  * Custom binary message protocol
  * CRC16 message validation
  * Signal-based event handling
  * Graceful shutdown with signal handling
  * Configurable logging levels
  * Cross-platform build system
Installed-Size: $INSTALLED_SIZE
EOF
}

# Create postinst script
create_postinst_script() {
    log_info "Creating postinst script..."
    
    cat > "$DEBIAN_DIR/postinst" << EOF
#!/bin/bash
set -e

# Enable and start the service
systemctl daemon-reload
systemctl enable uzs-bkk320-network-manager.service
systemctl start uzs-bkk320-network-manager.service

# Create log directory with proper permissions
mkdir -p /opt/Sensor-M/Bkk320/etc/networkManager

echo "uzs-bkk320-network-manager service enabled. Start it with: systemctl start uzs-bkk320-network-manager"
EOF

    chmod +x "$DEBIAN_DIR/postinst"
}

# Create prerm script
create_prerm_script() {
    log_info "Creating prerm script..."
    
    cat > "$DEBIAN_DIR/prerm" << EOF
#!/bin/bash
set -e

# Stop and disable the service
systemctl stop uzs-bkk320-network-manager.service || true
systemctl disable uzs-bkk320-network-manager.service || true
systemctl daemon-reload
EOF

    chmod +x "$DEBIAN_DIR/prerm"
}

# Create postrm script
create_postrm_script() {
    log_info "Creating postrm script..."
    
    cat > "$DEBIAN_DIR/postrm" << EOF
#!/bin/bash
set -e

case "\$1" in
    remove|purge)
        # Remove log files on purge
        if [ "\$1" = "purge" ]; then
            rm -rf /opt/Sensor-M/Bkk320/etc/networkManager/* || true
            rm -rf /opt/Sensor-M/Bkk320/etc/networkManager
        fi
        ;;
    upgrade|failed-upgrade|abort-install|abort-upgrade|disappear)
        ;;
    *)
        echo "postrm called with unknown argument '\$1'" >&2
        exit 1
        ;;
esac
EOF

    chmod +x "$DEBIAN_DIR/postrm"
}

# Build the .deb package
build_deb_package() {
    log_info "Building .deb package..."
    
    # Set proper permissions
    chmod +x "$INSTALL_DIR/bin/uzsBkk320NetworkManager"
    
    # Build the package
    dpkg-deb --build "$DEB_DIR" "$PROJECT_ROOT/debs/${PACKAGE_NAME}_${VERSION}_${ARCHITECTURE}.deb"
    
    if [ $? -eq 0 ]; then
        log_info "Package created successfully: ${PACKAGE_NAME}_${VERSION}_${ARCHITECTURE}.deb"
        
        # Show package info
        log_info "Package information:"
        dpkg-deb --info "$PROJECT_ROOT/debs/${PACKAGE_NAME}_${VERSION}_${ARCHITECTURE}.deb"
        
        # Show package contents
        log_info "Package contents:"
        dpkg-deb --contents "$PROJECT_ROOT/debs/${PACKAGE_NAME}_${VERSION}_${ARCHITECTURE}.deb"
    else
        log_error "Failed to create .deb package"
        exit 1
    fi
}

# Main execution
main() {
    log_info "Starting .deb package creation for uzsBkk320NetworkManager"
    log_info "Version: $VERSION"
    log_info "Architecture: $ARCHITECTURE"
    
    # Always rebuild target binary
    build_target
    
    # Create package
    create_package_structure
    copy_files
    create_systemd_service
    create_control_file
    create_postinst_script
    create_prerm_script
    create_postrm_script
    build_deb_package

    git tag -a "${VERSION}" -m "Version ${VERSION}"
    
    log_info "Package creation completed successfully!"
    log_info "Install with: sudo dpkg -i ${PACKAGE_NAME}_${VERSION}_${ARCHITECTURE}.deb"
}

# Run main function
main "$@"