#!/usr/bin/env python3
"""
Cross-platform build environment configuration script.
Automatically detects and installs missing dependencies.
"""

import os
import sys
import platform
import subprocess
import shutil
import logging
from pathlib import Path
from typing import List, Optional
import re

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class PlatformDetector:
    """Detect and provide platform-specific information."""
    
    @staticmethod
    def get_platform() -> str:
        """Get normalized platform name."""
        system = platform.system().lower()
        if system == 'darwin':
            return 'macos'
        elif system == 'windows':
            return 'windows'
        elif system == 'linux':
            return 'linux'
        else:
            raise RuntimeError(f"Unsupported platform: {system}")
    
    @staticmethod
    def get_arch() -> str:
        """Get architecture."""
        machine = platform.machine().lower()
        if machine in ['x86_64', 'amd64']:
            return 'x64'
        elif machine in ['aarch64', 'arm64']:
            return 'arm64'
        else:
            return machine
    
    @staticmethod
    def is_wsl() -> bool:
        """Check if running in WSL."""
        try:
            with open('/proc/version', 'r') as f:
                return 'microsoft' in f.read().lower() or 'wsl' in f.read().lower()
        except:
            return False
    
    @staticmethod
    def get_ubuntu_version() -> Optional[str]:
        """Get Ubuntu version if running on Ubuntu."""
        try:
            with open('/etc/os-release', 'r') as f:
                content = f.read()
                if 'Ubuntu' in content:
                    for line in content.split('\n'):
                        if line.startswith('VERSION_ID='):
                            return line.split('=')[1].strip('"')
        except:
            pass
        return None


class EnvironmentManager:
    """Manage persistent environment variables across platforms."""
    
    def __init__(self):
        self.platform = PlatformDetector.get_platform()
        self.shell_configs = self._get_shell_configs()
    
    def _get_shell_configs(self) -> List[Path]:
        """Get list of shell configuration files."""
        home = Path.home()
        configs = []
        
        if self.platform in ['linux', 'macos']:
            potential_configs = [
                home / '.bashrc',
                home / '.zshrc',
                home / '.profile',
                home / '.bash_profile'
            ]
            
            for config in potential_configs:
                if config.exists():
                    configs.append(config)
            
            if not configs:
                configs.append(home / '.bashrc')
        
        return configs
    
    def check_env_var_exists(self, var_name: str) -> bool:
        """Check if environment variable already exists."""
        if self.platform == 'windows':
            try:
                result = subprocess.run(
                    ['powershell', '-Command', f'[Environment]::GetEnvironmentVariable("{var_name}", "User")'],
                    capture_output=True, text=True, check=False
                )
                return bool(result.stdout.strip())
            except:
                return False
        else:
            # Check shell config files
            for config_file in self.shell_configs:
                if config_file.exists():
                    try:
                        with open(config_file, 'r') as f:
                            content = f.read()
                            pattern = rf'^\s*(export\s+)?{re.escape(var_name)}\s*='
                            if re.search(pattern, content, re.MULTILINE):
                                return True
                    except:
                        continue
            
            return var_name in os.environ
    
    def add_env_var(self, var_name: str, value: str, description: str = "") -> bool:
        """Add environment variable persistently."""
        if self.check_env_var_exists(var_name):
            logger.info(f"Environment variable {var_name} already exists, skipping")
            return True
        
        logger.info(f"Adding environment variable: {var_name}={value}")
        
        if self.platform == 'windows':
            return self._add_windows_env_var(var_name, value)
        else:
            return self._add_unix_env_var(var_name, value, description)
    
    def _add_windows_env_var(self, var_name: str, value: str) -> bool:
        """Add environment variable on Windows."""
        try:
            cmd = [
                'powershell', '-Command',
                f'[Environment]::SetEnvironmentVariable("{var_name}", "{value}", "User")'
            ]
            subprocess.run(cmd, check=True)
            os.environ[var_name] = value
            logger.info(f"✓ Windows environment variable {var_name} set")
            return True
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to set Windows environment variable {var_name}: {e}")
            return False
    
    def _add_unix_env_var(self, var_name: str, value: str, description: str = "") -> bool:
        """Add environment variable on Unix-like systems."""
        for config_file in self.shell_configs:
            try:
                config_file.touch()
                
                with open(config_file, 'r') as f:
                    content = f.read()
                
                pattern = rf'^\s*(export\s+)?{re.escape(var_name)}\s*='
                if re.search(pattern, content, re.MULTILINE):
                    continue
                
                lines_to_add = []
                if description:
                    lines_to_add.append(f"# {description}")
                lines_to_add.append(f'export {var_name}="{value}"')
                
                with open(config_file, 'a') as f:
                    if content and not content.endswith('\n'):
                        f.write('\n')
                    f.write('\n'.join(lines_to_add) + '\n')
                
                logger.info(f"✓ Added {var_name} to {config_file}")
                os.environ[var_name] = value
                return True
                
            except Exception as e:
                logger.warning(f"Failed to add {var_name} to {config_file}: {e}")
                continue
        
        return False
    
    def add_to_path(self, path_value: str, description: str = "") -> bool:
        """Add directory to PATH if not already present."""
        current_path = os.environ.get('PATH', '')
        
        if self.platform == 'windows':
            path_separator = ';'
        else:
            path_separator = ':'
        
        paths = current_path.split(path_separator)
        normalized_paths = [os.path.normpath(p) for p in paths]
        normalized_new_path = os.path.normpath(path_value)
        
        if normalized_new_path in normalized_paths:
            logger.info(f"Path {path_value} already in PATH, skipping")
            return True
        
        logger.info(f"Adding to PATH: {path_value}")
        
        if self.platform == 'windows':
            return self._add_windows_path(path_value)
        else:
            return self._add_unix_path(path_value, description)
    
    def _add_windows_path(self, path_value: str) -> bool:
        """Add directory to Windows PATH."""
        try:
            cmd = [
                'powershell', '-Command',
                '[Environment]::GetEnvironmentVariable("PATH", "User")'
            ]
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            current_user_path = result.stdout.strip()
            
            if current_user_path:
                new_path = f"{current_user_path};{path_value}"
            else:
                new_path = path_value
            
            cmd = [
                'powershell', '-Command',
                f'[Environment]::SetEnvironmentVariable("PATH", "{new_path}", "User")'
            ]
            subprocess.run(cmd, check=True)
            
            os.environ['PATH'] = f"{os.environ.get('PATH', '')};{path_value}"
            logger.info(f"✓ Added {path_value} to Windows PATH")
            return True
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to add {path_value} to Windows PATH: {e}")
            return False
    
    def _add_unix_path(self, path_value: str, description: str = "") -> bool:
        """Add directory to Unix PATH."""
        for config_file in self.shell_configs:
            try:
                config_file.touch()
                
                with open(config_file, 'r') as f:
                    content = f.read()
                
                if f'PATH="{path_value}:$PATH"' in content or f'PATH="$PATH:{path_value}"' in content:
                    continue
                
                lines_to_add = []
                if description:
                    lines_to_add.append(f"# {description}")
                lines_to_add.append(f'export PATH="{path_value}:$PATH"')
                
                with open(config_file, 'a') as f:
                    if content and not content.endswith('\n'):
                        f.write('\n')
                    f.write('\n'.join(lines_to_add) + '\n')
                
                logger.info(f"✓ Added {path_value} to PATH in {config_file}")
                current_path = os.environ.get('PATH', '')
                os.environ['PATH'] = f"{path_value}:{current_path}"
                return True
                
            except Exception as e:
                logger.warning(f"Failed to add {path_value} to PATH in {config_file}: {e}")
                continue
        
        return False
    
    def reload_environment(self) -> bool:
        """Reload environment variables by sourcing shell config files."""
        if self.platform == 'windows':
            logger.info("Environment variables are active for new processes on Windows")
            return True
        
        for config_file in self.shell_configs:
            if config_file.exists():
                try:
                    current_shell = os.environ.get('SHELL', '/bin/bash')
                    shell_name = Path(current_shell).name
                    
                    if shell_name in ['bash', 'sh']:
                        cmd = ['bash', '-c', f'source {config_file} && env']
                    elif shell_name == 'zsh':
                        cmd = ['zsh', '-c', f'source {config_file} && env']
                    else:
                        cmd = ['bash', '-c', f'source {config_file} && env']
                    
                    result = subprocess.run(cmd, capture_output=True, text=True, check=False)
                    
                    if result.returncode == 0:
                        env_lines = result.stdout.strip().split('\n')
                        updated_vars = []
                        
                        for line in env_lines:
                            if '=' in line:
                                key, value = line.split('=', 1)
                                if key in ['VCPKG_ROOT', 'PATH']:
                                    old_value = os.environ.get(key, '')
                                    if old_value != value:
                                        os.environ[key] = value
                                        updated_vars.append(key)
                        
                        if updated_vars:
                            logger.info(f"✓ Updated environment variables: {', '.join(updated_vars)}")
                            return True
                    
                    break
                    
                except Exception as e:
                    logger.debug(f"Failed to reload environment from {config_file}: {e}")
                    continue
        
        logger.info("To apply environment variables in current session, run:")
        for config_file in self.shell_configs[:1]:
            if config_file.exists():
                logger.info(f"  source {config_file}")
                break
        
        return False


class CommandRunner:
    """Execute system commands with proper error handling."""
    
    @staticmethod
    def run(cmd: List[str], shell: bool = False, capture: bool = False, 
            check: bool = True, cwd: Optional[str] = None) -> subprocess.CompletedProcess:
        """Run command and return result."""
        logger.info(f"Running: {' '.join(cmd) if isinstance(cmd, list) else cmd}")
        
        try:
            if capture:
                result = subprocess.run(
                    cmd, shell=shell, check=check, cwd=cwd,
                    capture_output=True, text=True
                )
            else:
                result = subprocess.run(cmd, shell=shell, check=check, cwd=cwd)
            
            if capture and result.stdout:
                logger.debug(f"Output: {result.stdout.strip()}")
                
            return result
        except subprocess.CalledProcessError as e:
            logger.error(f"Command failed with exit code {e.returncode}")
            if capture and e.stdout:
                logger.error(f"stdout: {e.stdout}")
            if capture and e.stderr:
                logger.error(f"stderr: {e.stderr}")
            raise
    
    @staticmethod
    def check_command_exists(cmd: str) -> bool:
        """Check if command exists in PATH."""
        return shutil.which(cmd) is not None
    
    @staticmethod
    def safe_install_packages(packages: List[str], use_sudo: bool = True) -> List[str]:
        """Safely install packages, returning list of failed packages."""
        failed_packages = []
        
        # First try to install all packages at once
        cmd_prefix = ['sudo', 'apt-get', 'install', '-y'] if use_sudo else ['apt-get', 'install', '-y']
        try:
            CommandRunner.run(cmd_prefix + packages)
            logger.info(f"✓ Successfully installed all packages: {', '.join(packages)}")
            return []
        except subprocess.CalledProcessError:
            logger.warning("Batch installation failed, trying individual packages...")
        
        # If batch installation fails, try individual packages
        for package in packages:
            try:
                CommandRunner.run(cmd_prefix + [package])
                logger.debug(f"✓ {package} installed")
            except subprocess.CalledProcessError:
                logger.warning(f"✗ Failed to install {package}")
                failed_packages.append(package)
        
        return failed_packages


class DependencyChecker:
    """Check if dependencies are already installed."""
    
    def __init__(self):
        self.platform = PlatformDetector.get_platform()
    
    def check_basic_tools(self) -> dict:
        """Check basic development tools."""
        tools = {
            'git': CommandRunner.check_command_exists('git'),
            'cmake': CommandRunner.check_command_exists('cmake'),
            'ninja': CommandRunner.check_command_exists('ninja'),
            'python3': CommandRunner.check_command_exists('python3'),
            'pip3': CommandRunner.check_command_exists('pip3'),
        }
        
        if self.platform == 'linux':
            tools.update({
                'gcc': CommandRunner.check_command_exists('gcc'),
                'g++': CommandRunner.check_command_exists('g++'),
                'pkg-config': CommandRunner.check_command_exists('pkg-config'),
                'make': CommandRunner.check_command_exists('make'),
                'autoconf': CommandRunner.check_command_exists('autoconf'),
                'automake': CommandRunner.check_command_exists('automake'),
                'libtool': CommandRunner.check_command_exists('libtool'),
                'meson': CommandRunner.check_command_exists('meson'),
            })
            
            # Check Python modules
            tools.update(self._check_python_modules())
        
        return tools
    
    def _check_python_modules(self) -> dict:
        """Check if important Python modules are available."""
        modules_to_check = ['jinja2', 'packaging', 'setuptools', 'wheel']
        module_status = {}
        
        for module in modules_to_check:
            try:
                result = CommandRunner.run(
                    ['python3', '-c', f'import {module}'],
                    capture=True, check=False
                )
                module_status[f'python3-{module}'] = result.returncode == 0
            except:
                module_status[f'python3-{module}'] = False
        
        return module_status
    
    def check_vcpkg(self, project_root: Path) -> bool:
        """Check if vcpkg is installed and working."""
        vcpkg_root = project_root / 'tools' / 'vcpkg'
        if not vcpkg_root.exists():
            return False
        
        vcpkg_exe = 'vcpkg.exe' if self.platform == 'windows' else 'vcpkg'
        vcpkg_path = vcpkg_root / vcpkg_exe
        
        if not vcpkg_path.exists():
            return False
        
        try:
            result = CommandRunner.run([str(vcpkg_path), 'version'], capture=True, check=False)
            return result.returncode == 0
        except:
            return False


class LinuxDependencyInstaller:
    """Install dependencies on Linux."""
    
    def __init__(self):
        self.package_manager = self._detect_package_manager()
        self.is_wsl = PlatformDetector.is_wsl()
        self.ubuntu_version = PlatformDetector.get_ubuntu_version()
        self.skip_python_modules = False  # Add this line - this was missing
    
    def _detect_package_manager(self) -> str:
        """Detect available package manager."""
        if CommandRunner.check_command_exists('apt-get'):
            return 'apt'
        elif CommandRunner.check_command_exists('yum'):
            return 'yum'
        elif CommandRunner.check_command_exists('dnf'):
            return 'dnf'
        elif CommandRunner.check_command_exists('pacman'):
            return 'pacman'
        else:
            raise RuntimeError("No supported package manager found")
    
    def install_basic_dependencies(self):
        """Install basic build dependencies."""
        logger.info("Installing basic build dependencies...")
        
        if self.package_manager == 'apt':
            # Update first
            CommandRunner.run(['sudo', 'apt-get', 'update'])
            
            # Fix broken packages if any
            CommandRunner.run(['sudo', 'apt-get', 'install', '-f', '-y'], check=False)
            
            # Core build tools - these should work on all Ubuntu versions
            core_packages = [
                'build-essential', 'cmake', 'ninja-build', 'pkg-config',
                'curl', 'zip', 'unzip', 'tar', 'git'
            ]
            
            # Autotools and development libraries
            autotools_packages = [
                'autoconf', 'automake', 'libtool', 'autoconf-archive',
                'autotools-dev', 'autogen', 'gettext', 'gperf'
            ]
            
            # Essential development libraries
            essential_dev_libraries = [
                'libudev-dev', 'libusb-1.0-0-dev', 'libssl-dev', 'zlib1g-dev',
                'libx11-dev', 'libxext-dev', 'libxfixes-dev', 'libxi-dev', 
                'libxrandr-dev', 'libxss-dev', 'libdbus-1-dev', 'libgcrypt20-dev',
                'libsystemd-dev', 'libcap-dev', 'libmount-dev'
            ]
            
            # Python development tools and modules
            python_packages = []
            
            # Handle Python packages based on Ubuntu version
            if self.ubuntu_version:
                try:
                    version_float = float(self.ubuntu_version)
                    
                    if version_float >= 22.04:
                        # Ubuntu 22.04+ - python3-distutils has been removed
                        python_packages = [
                            'python3-dev', 'python3-pip', 'python3-setuptools', 'python3-wheel',
                            'python3-venv', 'python3-full'  # python3-full includes distutils functionality
                        ]
                    else:
                        # Older Ubuntu versions
                        python_packages = [
                            'python3-dev', 'python3-pip', 'python3-setuptools', 'python3-wheel',
                            'python3-venv', 'python3-distutils'
                        ]
                except ValueError:
                    # If we can't parse version, use the safer modern approach
                    python_packages = [
                        'python3-dev', 'python3-pip', 'python3-setuptools', 'python3-wheel',
                        'python3-venv', 'python3-full'
                    ]
            else:
                # Default to modern packages if we can't detect Ubuntu version
                python_packages = [
                    'python3-dev', 'python3-pip', 'python3-setuptools', 'python3-wheel',
                    'python3-venv', 'python3-full'
                ]
            
            # Additional tools
            extra_tools = [
                'flex', 'bison', 'nasm', 'yasm', 'texinfo', 'help2man',
                'wget', 'rsync', 'file', 'meson'
            ]
            
            # Version-specific packages that might not exist in newer Ubuntu
            version_specific_packages = []
            
            # Handle Ubuntu version-specific packages
            if self.ubuntu_version:
                try:
                    version_float = float(self.ubuntu_version)
                    
                    if version_float >= 24.04:
                        # Ubuntu 24.04+ packages
                        version_specific_packages.extend([
                            'libasound2t64',  # Instead of libasound2
                            'libglib2.0-dev', 'libgtk-3-dev',
                            'libxtst6', 'libatspi2.0-0', 'libdrm2',
                            'libxcomposite1', 'libxdamage1', 'libgbm1',
                            'libnss3'
                        ])
                    else:
                        # Older Ubuntu versions
                        version_specific_packages.extend([
                            'libasound2-dev',  # Try the dev package instead
                            'libglib2.0-dev', 'libgtk-3-dev',
                            'libxtst6', 'libatspi2.0-0', 'libdrm2',
                            'libxcomposite1', 'libxdamage1', 'libgbm1',
                            'libnss3'
                        ])
                        
                        # These packages might not exist in newer versions
                        legacy_packages = ['libgconf-2-4', 'libappindicator1']
                        version_specific_packages.extend(legacy_packages)
                        
                except ValueError:
                    # If we can't parse the version, use conservative approach
                    version_specific_packages.extend([
                        'libglib2.0-dev', 'libgtk-3-dev',
                        'libxtst6', 'libatspi2.0-0', 'libdrm2',
                        'libxcomposite1', 'libxdamage1', 'libgbm1',
                        'libnss3'
                    ])
            
            # Install packages in batches to handle failures gracefully
            package_groups = [
                ("Core build tools", core_packages),
                ("Autotools", autotools_packages),
                ("Essential development libraries", essential_dev_libraries),
                ("Python tools", python_packages),
                ("Extra tools", extra_tools),
                ("Version-specific packages", version_specific_packages)
            ]
            
            all_failed_packages = []
            
            for group_name, packages in package_groups:
                if not packages:
                    continue
                    
                logger.info(f"Installing {group_name}...")
                failed_packages = CommandRunner.safe_install_packages(packages)
                
                if failed_packages:
                    logger.warning(f"Failed to install from {group_name}: {', '.join(failed_packages)}")
                    all_failed_packages.extend(failed_packages)
                else:
                    logger.info(f"✓ {group_name} installed successfully")
            
            # Report overall status
            if all_failed_packages:
                logger.warning(f"Some packages failed to install: {', '.join(set(all_failed_packages))}")
                logger.info("This is normal for newer Ubuntu versions where some packages have been renamed or removed")
            else:
                logger.info("✓ All package groups installed successfully")
            
            # Install Python modules that are commonly needed for building
            self._install_python_modules()
            
            # Clean up package cache to save space
            try:
                CommandRunner.run(['sudo', 'apt-get', 'autoremove', '-y'], check=False)
                CommandRunner.run(['sudo', 'apt-get', 'autoclean'], check=False)
            except:
                pass
            
        elif self.package_manager in ['yum', 'dnf']:
            packages = [
                'gcc', 'gcc-c++', 'cmake', 'ninja-build', 'pkgconfig',
                'curl', 'zip', 'unzip', 'tar', 'git', 'autoconf', 'automake',
                'libtool', 'gettext-devel', 'flex', 'bison', 'nasm',
                'systemd-devel', 'libusbx-devel', 'openssl-devel', 'zlib-devel',
                'python3-devel', 'python3-pip', 'meson'
            ]
            
            failed_packages = CommandRunner.safe_install_packages(packages)
            if failed_packages:
                logger.warning(f"Failed to install: {', '.join(failed_packages)}")
            
            self._install_python_modules()
        
        elif self.package_manager == 'pacman':
            packages = [
                'base-devel', 'cmake', 'ninja', 'pkgconf', 'curl', 'zip',
                'unzip', 'tar', 'git', 'autoconf', 'automake', 'libtool',
                'gettext', 'flex', 'bison', 'nasm', 'systemd', 'libusb',
                'openssl', 'zlib', 'python', 'python-pip', 'meson'
            ]
            
            for package in packages:
                try:
                    CommandRunner.run(['sudo', 'pacman', '-S', '--noconfirm', package])
                except subprocess.CalledProcessError:
                    logger.warning(f"Failed to install {package}, continuing...")
            
            self._install_python_modules()
    
    def _install_python_modules(self):
        """Install commonly needed Python modules for building."""
        logger.info("Installing Python modules needed for building...")
        
        # Common Python modules needed by build systems
        python_modules = [
            'jinja2',          # Template engine (needed by meson, systemd, etc.)
            'packaging',       # Package handling utilities
            'setuptools',      # Python package development
            'wheel',           # Binary package format
            'pyyaml',          # YAML parsing
            'requests',        # HTTP requests
            'six',             # Python 2/3 compatibility
            'markupsafe',      # HTML/XML markup safe strings
            'certifi',         # SSL certificates
            'urllib3',         # HTTP client
            'charset-normalizer', # Character encoding detection
            'idna',            # Internationalized domain names
            'mako',            # Template library
            'pygments',        # Syntax highlighting
            'docutils',        # Documentation utilities
        ]
        
        # First try to install available packages via apt
        logger.info("Trying to install Python modules via apt...")
        apt_python_packages = []
        for module in python_modules:
            # Common apt package naming patterns
            potential_packages = [
                f'python3-{module}',
                f'python3-{module.replace("_", "-")}',
                f'python3-{module.replace("-", "")}',
            ]
            
            # Add some specific mappings for packages with different names
            package_mappings = {
                'charset-normalizer': ['python3-charset-normalizer'],
                'markupsafe': ['python3-markupsafe'],
                'pyyaml': ['python3-yaml'],
                'urllib3': ['python3-urllib3'],
                'certifi': ['python3-certifi'],
                'requests': ['python3-requests'],
                'six': ['python3-six'],
                'jinja2': ['python3-jinja2'],
                'packaging': ['python3-packaging'],
                'setuptools': ['python3-setuptools'],
                'wheel': ['python3-wheel'],
                'mako': ['python3-mako'],
                'pygments': ['python3-pygments'],
                'docutils': ['python3-docutils'],
                'idna': ['python3-idna']
            }
            
            if module in package_mappings:
                apt_python_packages.extend(package_mappings[module])
            else:
                apt_python_packages.extend(potential_packages)
        
        # Remove duplicates and try to install
        unique_apt_packages = list(set(apt_python_packages))
        
        # Try to install apt packages first
        logger.info(f"Attempting to install Python modules via apt: {unique_apt_packages}")
        failed_apt_packages = CommandRunner.safe_install_packages(unique_apt_packages)
        
        if failed_apt_packages:
            logger.info(f"Some apt packages failed: {failed_apt_packages}")
        
        # Check which modules are still missing after apt installation
        missing_modules = []
        for module in python_modules:
            try:
                result = CommandRunner.run(
                    ['python3', '-c', f'import {module}'],
                    capture=True, check=False
                )
                if result.returncode != 0:
                    missing_modules.append(module)
            except:
                missing_modules.append(module)
        
        if not missing_modules:
            logger.info("✓ All Python modules are available via system packages")
            return
        
        logger.info(f"Still missing Python modules: {missing_modules}")
        
        # For missing modules, try different pip installation strategies
        self._install_missing_python_modules(missing_modules)
    
    def _install_missing_python_modules(self, modules: List[str]):
        """Try different strategies to install missing Python modules."""
        logger.info("Trying alternative Python module installation methods...")
        
        # Strategy 1: Try --break-system-packages (for externally managed environments)
        logger.info("Strategy 1: Using --break-system-packages flag...")
        strategy1_failed = []
        for module in modules:
            try:
                result = CommandRunner.run([
                    'python3', '-m', 'pip', 'install', '--break-system-packages', module
                ], capture=True, check=False)
                
                if result.returncode == 0:
                    logger.debug(f"✓ {module} installed with --break-system-packages")
                else:
                    strategy1_failed.append(module)
                    logger.debug(f"✗ Failed to install {module} with --break-system-packages")
            except:
                strategy1_failed.append(module)
        
        if not strategy1_failed:
            logger.info("✓ All modules installed with --break-system-packages")
            return
        
        # Strategy 2: Try using pipx (if available)
        if CommandRunner.check_command_exists('pipx'):
            logger.info("Strategy 2: Using pipx for application packages...")
            strategy2_failed = []
            for module in strategy1_failed:
                try:
                    result = CommandRunner.run([
                        'pipx', 'install', module
                    ], capture=True, check=False)
                    
                    if result.returncode == 0:
                        logger.debug(f"✓ {module} installed with pipx")
                    else:
                        strategy2_failed.append(module)
                except:
                    strategy2_failed.append(module)
            
            strategy1_failed = strategy2_failed
        else:
            logger.info("pipx not available, trying to install it...")
            try:
                CommandRunner.run(['sudo', 'apt-get', 'install', '-y', 'pipx'], check=False)
            except:
                logger.warning("Failed to install pipx")
        
        # Strategy 3: Create a user virtual environment
        if strategy1_failed:
            logger.info("Strategy 3: Creating user virtual environment...")
            try:
                self._create_user_venv_and_install(strategy1_failed)
            except Exception as e:
                logger.warning(f"Virtual environment strategy failed: {e}")
        
        # Strategy 4: Install python3-full and retry
        if strategy1_failed:
            logger.info("Strategy 4: Installing python3-full and retrying...")
            try:
                CommandRunner.run(['sudo', 'apt-get', 'install', '-y', 'python3-full'], check=False)
                
                # Retry with --user flag
                for module in strategy1_failed:
                    try:
                        CommandRunner.run([
                            'python3', '-m', 'pip', 'install', '--user', module
                        ], check=False)
                        logger.debug(f"✓ {module} installed with --user after python3-full")
                    except:
                        logger.debug(f"✗ Still failed to install {module}")
            except:
                logger.warning("Failed to install python3-full")
        
        # Final verification
        final_missing = []
        for module in modules:
            try:
                result = CommandRunner.run(
                    ['python3', '-c', f'import {module}'],
                    capture=True, check=False
                )
                if result.returncode != 0:
                    final_missing.append(module)
            except:
                final_missing.append(module)
        
        if final_missing:
            logger.warning(f"Still missing Python modules: {final_missing}")
            logger.info("These modules may not be critical for basic building, continuing...")
        else:
            logger.info("✓ All Python modules are now available")
    
    def _create_user_venv_and_install(self, modules: List[str]):
        """Create a user virtual environment and install modules there."""
        import tempfile
        import shutil
        
        logger.info("Creating temporary virtual environment for module installation...")
        
        # Create a temporary virtual environment
        with tempfile.TemporaryDirectory() as temp_dir:
            venv_path = Path(temp_dir) / "temp_venv"
            
            # Create venv
            CommandRunner.run(['python3', '-m', 'venv', str(venv_path)])
            
            # Install modules in venv
            venv_pip = venv_path / 'bin' / 'pip'
            for module in modules:
                try:
                    CommandRunner.run([str(venv_pip), 'install', module], check=False)
                    logger.debug(f"✓ {module} installed in temporary venv")
                except:
                    logger.debug(f"✗ Failed to install {module} in temporary venv")
            
            # Copy installed packages to user site-packages (this is a hack but sometimes works)
            try:
                import site
                user_site = site.getusersitepackages()
                if not os.path.exists(user_site):
                    os.makedirs(user_site, exist_ok=True)
                
                venv_site = venv_path / 'lib' / 'python3.12' / 'site-packages'  # Adjust version as needed
                if not venv_site.exists():
                    # Try to find the correct site-packages directory
                    for version_dir in (venv_path / 'lib').glob('python*'):
                        potential_site = version_dir / 'site-packages'
                        if potential_site.exists():
                            venv_site = potential_site
                            break
                
                if venv_site.exists():
                    for module in modules:
                        # Find module directories/files in venv
                        for item in venv_site.glob(f'{module}*'):
                            if item.is_dir() or item.suffix in ['.py', '.pth']:
                                dest = Path(user_site) / item.name
                                if item.is_dir():
                                    if dest.exists():
                                        shutil.rmtree(dest)
                                    shutil.copytree(item, dest)
                                else:
                                    shutil.copy2(item, dest)
                                logger.debug(f"✓ Copied {item.name} to user site-packages")
                
            except Exception as e:
                logger.debug(f"Failed to copy venv packages to user site: {e}")

    def install_code_quality_tools(self):
        """Install code quality tools like clang-format, cppcheck, cpplint."""
        logger.info("Installing code quality tools...")
        
        if self.package_manager == 'apt':
            # Code quality tools
            code_quality_packages = [
                'clang-format',      # Code formatter
                'cppcheck',          # Static analysis tool
                'clang-tidy',        # Static analysis and linting
                'iwyu',              # Include what you use (if available)
                'flawfinder',        # Security-focused static analysis
                'vera++',            # Style checker (if available)
            ]
            
            logger.info("Installing code quality packages...")
            failed_packages = CommandRunner.safe_install_packages(code_quality_packages)
            
            if failed_packages:
                logger.warning(f"Failed to install code quality tools: {', '.join(failed_packages)}")
            else:
                logger.info("✓ Code quality tools installed successfully")
            
            # Install cpplint via pip since it's not always available as apt package
            self._install_cpplint()
            
        elif self.package_manager in ['yum', 'dnf']:
            packages = [
                'clang-tools-extra',  # Contains clang-format, clang-tidy
                'cppcheck',
            ]
            
            failed_packages = CommandRunner.safe_install_packages(packages)
            if failed_packages:
                logger.warning(f"Failed to install code quality tools: {', '.join(failed_packages)}")
            
            self._install_cpplint()
        
        elif self.package_manager == 'pacman':
            packages = [
                'clang',             # Contains clang-format
                'cppcheck',
            ]
            
            for package in packages:
                try:
                    CommandRunner.run(['sudo', 'pacman', '-S', '--noconfirm', package])
                except subprocess.CalledProcessError:
                    logger.warning(f"Failed to install {package}, continuing...")
            
            self._install_cpplint()
    
    def _install_cpplint(self):
        """Install cpplint via pip."""
        logger.info("Installing cpplint via pip...")
        
        try:
            # Try different pip installation strategies
            strategies = [
                ['python3', '-m', 'pip', 'install', '--user', 'cpplint'],
                ['python3', '-m', 'pip', 'install', '--break-system-packages', 'cpplint'],
                ['pip3', 'install', '--user', 'cpplint'],
            ]
            
            for strategy in strategies:
                try:
                    CommandRunner.run(strategy, check=False)
                    # Verify installation
                    result = CommandRunner.run(['cpplint', '--version'], capture=True, check=False)
                    if result.returncode == 0:
                        logger.info("✓ cpplint installed successfully")
                        return
                except:
                    continue
            
            logger.warning("Failed to install cpplint via pip, trying apt...")
            try:
                CommandRunner.run(['sudo', 'apt-get', 'install', '-y', 'python3-cpplint'], check=False)
            except:
                logger.warning("Could not install cpplint")
                
        except Exception as e:
            logger.warning(f"Failed to install cpplint: {e}")


class MacOSDependencyInstaller:
    """Install dependencies on macOS."""
    
    def __init__(self):
        self.is_homebrew_available = CommandRunner.check_command_exists('brew')
        self.skip_python_modules = False  # Add this line - this was missing
    
    def _install_homebrew(self):
        """Install Homebrew if not available."""
        if self.is_homebrew_available:
            return
        
        logger.info("Installing Homebrew...")
        install_script = '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
        try:
            CommandRunner.run(['bash', '-c', install_script])
            self.is_homebrew_available = True
            
            # Add Homebrew to PATH for current session
            if PlatformDetector.get_arch() == 'arm64':
                brew_path = '/opt/homebrew/bin'
            else:
                brew_path = '/usr/local/bin'
            
            current_path = os.environ.get('PATH', '')
            if brew_path not in current_path:
                os.environ['PATH'] = f"{brew_path}:{current_path}"
                
        except Exception as e:
            logger.error(f"Failed to install Homebrew: {e}")
            raise
    
    def _emergency_fix_aclocal(self):
        """Emergency fix for aclocal not being found - for CI environments."""
        logger.info("Applying emergency fix for aclocal...")
        
        homebrew_prefix = '/opt/homebrew' if PlatformDetector.get_arch() == 'arm64' else '/usr/local'
        
        # Common locations where aclocal might be hiding
        possible_aclocal_locations = [
            f"{homebrew_prefix}/opt/automake/bin/aclocal",
            f"{homebrew_prefix}/Cellar/automake/*/bin/aclocal",
        ]
        
        # Find aclocal in Cellar if not in expected location
        if not os.path.exists(f"{homebrew_prefix}/opt/automake/bin/aclocal"):
            import glob
            cellar_aclocal = glob.glob(f"{homebrew_prefix}/Cellar/automake/*/bin/aclocal")
            if cellar_aclocal:
                possible_aclocal_locations.extend(cellar_aclocal)
        
        # Try to create symlinks
        target_aclocal = f"{homebrew_prefix}/bin/aclocal"
        
        for source_path in possible_aclocal_locations:
            if '*' in source_path:
                # Expand glob pattern
                import glob
                expanded = glob.glob(source_path)
                for expanded_path in expanded:
                    if os.path.exists(expanded_path):
                        try:
                            if os.path.exists(target_aclocal):
                                os.remove(target_aclocal)
                            os.symlink(expanded_path, target_aclocal)
                            logger.info(f"✓ Emergency aclocal symlink created: {target_aclocal} -> {expanded_path}")
                            return True
                        except Exception as e:
                            logger.debug(f"✗ Failed to create emergency symlink: {e}")
            else:
                if os.path.exists(source_path):
                    try:
                        if os.path.exists(target_aclocal):
                            os.remove(target_aclocal)
                        os.symlink(source_path, target_aclocal)
                        logger.info(f"✓ Emergency aclocal symlink created: {target_aclocal} -> {source_path}")
                        return True
                    except Exception as e:
                        logger.debug(f"✗ Failed to create emergency symlink: {e}")
        
        # If symlink fails, try copying the file
        for source_path in possible_aclocal_locations:
            if '*' in source_path:
                import glob
                expanded = glob.glob(source_path)
                for expanded_path in expanded:
                    if os.path.exists(expanded_path):
                        try:
                            import shutil
                            shutil.copy2(expanded_path, target_aclocal)
                            os.chmod(target_aclocal, 0o755)
                            logger.info(f"✓ Emergency aclocal copy created: {expanded_path} -> {target_aclocal}")
                            return True
                        except Exception as e:
                            logger.debug(f"✗ Failed to copy aclocal: {e}")
            else:
                if os.path.exists(source_path):
                    try:
                        import shutil
                        shutil.copy2(source_path, target_aclocal)
                        os.chmod(target_aclocal, 0o755)
                        logger.info(f"✓ Emergency aclocal copy created: {source_path} -> {target_aclocal}")
                        return True
                    except Exception as e:
                        logger.debug(f"✗ Failed to copy aclocal: {e}")
        
        logger.warning("Emergency aclocal fix failed - could not locate or link aclocal")
        return False

    def install_basic_dependencies(self):
        """Install basic build dependencies for macOS."""
        logger.info("Installing basic build dependencies for macOS...")
        
        # Ensure Homebrew is available
        if not self.is_homebrew_available:
            self._install_homebrew()
        
        # Update Homebrew
        try:
            CommandRunner.run(['brew', 'update'])
        except Exception as e:
            logger.warning(f"Failed to update Homebrew: {e}")
        
        # Core development tools
        core_packages = [
            'cmake',
            'ninja', 
            'git',
            'pkg-config'
        ]
        
        # Essential autotools - critical for building many vcpkg packages
        autotools_packages = [
            'autoconf',          # Contains autoconf, autoheader, autom4te, etc.
            'automake',          # Contains aclocal, automake, etc.
            'libtool',           # Contains libtool, libtoolize, etc.
            'autoconf-archive',  # Additional autoconf macros - CRITICAL for ICU
            'm4',                # Macro processor used by autotools
            'gettext',           # Internationalization tools
        ]
        
        # Build tools and utilities
        build_tools = [
            'make',          # GNU make
            'flex',          # Fast lexical analyzer
            'bison',         # Parser generator
            'texinfo',       # Documentation system
            'help2man',      # Generate man pages
            'nasm',          # Assembler
            'yasm',          # Assembler
        ]
        
        # Python development
        python_packages = [
            'python@3.11',   # Python 3.11
            'python@3.12',   # Python 3.12 (backup)
        ]
        
        # Additional utilities
        utility_packages = [
            'wget',
            'curl',
            'rsync',
            'zip',
            'unzip',
            'gnu-tar',       # GNU tar (some builds expect GNU tar)
            'gnu-sed',       # GNU sed (some builds expect GNU sed)
            'coreutils',     # GNU core utilities
            'findutils',     # GNU find, xargs, etc.
            'meson',         # Modern build system
        ]
        
        # SSL/TLS and compression libraries
        library_packages = [
            'openssl@3',
            'zlib',
            'bzip2',
            'xz',
            'lz4',
            'zstd',
        ]
        
        # Install packages in groups
        package_groups = [
            ("Core development tools", core_packages),
            ("Autotools (critical for ICU and other packages)", autotools_packages),
            ("Build tools", build_tools),
            ("Python", python_packages),
            ("Utilities", utility_packages),
            ("Libraries", library_packages)
        ]
        
        all_failed_packages = []
        
        for group_name, packages in package_groups:
            if not packages:
                continue
            
            logger.info(f"Installing {group_name}...")
            failed_packages = self._safe_brew_install(packages)
            
            if failed_packages:
                logger.warning(f"Failed to install from {group_name}: {', '.join(failed_packages)}")
                all_failed_packages.extend(failed_packages)
            else:
                logger.info(f"✓ {group_name} installed successfully")
        
        # Install Python modules
        self._install_python_modules()

        # Reinstall autotools to ensure proper linking
        self.reinstall_autotools()
        
        # Link essential tools to ensure they're in PATH
        self._link_essential_tools()
        
        # --- relink automake aclocal ---
        try:
            CommandRunner.run(['brew', 'unlink', 'automake'], check=False)
            CommandRunner.run(['brew', 'link', '--force', '--overwrite', 'automake'], check=False)
            logger.info("✓ automake relinked")
        except Exception as e:
            logger.warning(f"Failed to relink automake: {e}")

        # check aclocal validity
        aclocal_path = '/usr/local/bin/aclocal' if PlatformDetector.get_arch() != 'arm64' else '/opt/homebrew/bin/aclocal'
        automake_aclocal = '/usr/local/opt/automake/bin/aclocal' if PlatformDetector.get_arch() != 'arm64' else '/opt/homebrew/opt/automake/bin/aclocal'
        if not os.path.exists(aclocal_path) and os.path.exists(automake_aclocal):
            try:
                os.symlink(automake_aclocal, aclocal_path)
                logger.info(f"✓ Created symlink for aclocal: {aclocal_path} -> {automake_aclocal}")
            except Exception as e:
                logger.warning(f"Failed to create symlink for aclocal: {e}")
        elif os.path.exists(aclocal_path):
            logger.info("✓ aclocal is available")
        else:
            logger.warning("aclocal not found, applying emergency fix...")
            self._emergency_fix_aclocal()
            
        # Final verification
        if CommandRunner.check_command_exists('aclocal'):
            logger.info("✓ aclocal is now available")
        else:
            logger.error("✗ aclocal is still not available after all fixes")

        # Report status
        if all_failed_packages:
            logger.warning(f"Some packages failed to install: {', '.join(set(all_failed_packages))}")
        else:
            logger.info("✓ All package groups installed successfully")
    
    def _safe_brew_install(self, packages: List[str]) -> List[str]:
        """Safely install brew packages, returning list of failed packages."""
        failed_packages = []
        
        # Try to install packages individually to handle conflicts
        for package in packages:
            try:
                # Check if already installed
                result = CommandRunner.run(['brew', 'list', package], capture=True, check=False)
                if result.returncode == 0:
                    logger.debug(f"✓ {package} already installed")
                    continue
                
                # Install the package
                CommandRunner.run(['brew', 'install', package], check=False)
                
                # Verify installation
                result = CommandRunner.run(['brew', 'list', package], capture=True, check=False)
                if result.returncode == 0:
                    logger.debug(f"✓ {package} installed successfully")
                else:
                    failed_packages.append(package)
                    logger.warning(f"✗ {package} installation verification failed")
                    
            except subprocess.CalledProcessError:
                # Try with --force-bottle if regular install fails
                try:
                    logger.debug(f"Retrying {package} with --force-bottle...")
                    CommandRunner.run(['brew', 'install', '--force-bottle', package], check=False)
                    logger.debug(f"✓ {package} installed with --force-bottle")
                except subprocess.CalledProcessError:
                    failed_packages.append(package)
                    logger.warning(f"✗ Failed to install {package}")
            except Exception as e:
                failed_packages.append(package)
                logger.warning(f"✗ Failed to install {package}: {e}")
        
        return failed_packages
    
    def _link_essential_tools(self):
        """Ensure essential tools are properly linked and in PATH."""
        logger.info("Linking essential development tools...")
        
        # Tools that need to be linked/available
        essential_tools = [
            'autoconf',
            'automake', 
            'aclocal',    # This is the missing tool from the error
            'libtool',
            'libtoolize',
            'm4',
            'autoheader',
            'autoreconf',
            'autom4te',
            'make',
            'cmake',
            'ninja',
            'pkg-config'
        ]
        
        # First, try to link packages via brew
        packages_to_link = ['autoconf', 'automake', 'libtool', 'm4', 'gettext']
        for package in packages_to_link:
            try:
                # Force link packages
                CommandRunner.run(['brew', 'unlink', package], check=False)
                CommandRunner.run(['brew', 'link', '--force', '--overwrite', package], check=False)
                logger.debug(f"✓ Force-linked {package}")
            except:
                logger.debug(f"✗ Failed to force-link {package}")
        
        # Try to link tools if they're not in PATH
        missing_tools = []
        for tool in essential_tools:
            if not CommandRunner.check_command_exists(tool):
                missing_tools.append(tool)
        
        if missing_tools:
            logger.warning(f"Some essential tools are still missing: {missing_tools}")
            
            # Add comprehensive Homebrew paths to current session PATH
            homebrew_paths = []
            if PlatformDetector.get_arch() == 'arm64':
                homebrew_paths = [
                    '/opt/homebrew/bin',
                    '/opt/homebrew/sbin',
                    '/opt/homebrew/opt/autoconf/bin',
                    '/opt/homebrew/opt/automake/bin',
                    '/opt/homebrew/opt/libtool/bin',
                    '/opt/homebrew/opt/m4/bin',
                    '/opt/homebrew/opt/gettext/bin',
                    '/opt/homebrew/opt/make/libexec/gnubin',
                    '/opt/homebrew/share/aclocal',  # Add aclocal share directory
                ]
            else:
                homebrew_paths = [
                    '/usr/local/bin',
                    '/usr/local/sbin', 
                    '/usr/local/opt/autoconf/bin',
                    '/usr/local/opt/automake/bin',
                    '/usr/local/opt/libtool/bin',
                    '/usr/local/opt/m4/bin',
                    '/usr/local/opt/gettext/bin',
                    '/usr/local/opt/make/libexec/gnubin',
                    '/usr/local/share/aclocal',  # Add aclocal share directory
                ]
            
            # Add these paths to current session
            current_path = os.environ.get('PATH', '')
            path_parts = current_path.split(':')
            
            for hb_path in homebrew_paths:
                if os.path.exists(hb_path) and hb_path not in path_parts:
                    path_parts.insert(0, hb_path)
            
            new_path = ':'.join(path_parts)
            os.environ['PATH'] = new_path
            logger.info(f"Updated PATH with comprehensive Homebrew tool paths")
            
            # Also set ACLOCAL_PATH for aclocal to find macro files
            aclocal_paths = []
            if PlatformDetector.get_arch() == 'arm64':
                aclocal_paths = [
                    '/opt/homebrew/share/aclocal',
                    '/opt/homebrew/opt/autoconf/share/aclocal',
                    '/opt/homebrew/opt/automake/share/aclocal',
                    '/opt/homebrew/opt/libtool/share/aclocal',
                    '/opt/homebrew/opt/gettext/share/aclocal',
                ]
            else:
                aclocal_paths = [
                    '/usr/local/share/aclocal',
                    '/usr/local/opt/autoconf/share/aclocal',
                    '/usr/local/opt/automake/share/aclocal',
                    '/usr/local/opt/libtool/share/aclocal',
                    '/usr/local/opt/gettext/share/aclocal',
                ]
            
            # Set ACLOCAL_PATH
            existing_aclocal_path = os.environ.get('ACLOCAL_PATH', '')
            valid_aclocal_paths = [p for p in aclocal_paths if os.path.exists(p)]
            
            if valid_aclocal_paths:
                if existing_aclocal_path:
                    new_aclocal_path = ':'.join(valid_aclocal_paths + [existing_aclocal_path])
                else:
                    new_aclocal_path = ':'.join(valid_aclocal_paths)
                
                os.environ['ACLOCAL_PATH'] = new_aclocal_path
                logger.info(f"Set ACLOCAL_PATH: {new_aclocal_path}")
            
            # Create symlinks for missing tools if they exist in Homebrew directories
            self._create_tool_symlinks(missing_tools)
            
            # Verify tools again
            still_missing = []
            for tool in missing_tools:
                if not CommandRunner.check_command_exists(tool):
                    still_missing.append(tool)
                else:
                    logger.debug(f"✓ {tool} now available")
            
            if still_missing:
                logger.error(f"Critical tools still missing: {still_missing}")
                logger.error("This may cause vcpkg package builds to fail")
                
                # Try one more strategy - check if tools exist in Homebrew but aren't linked
                self._diagnose_missing_tools(still_missing)
        else:
            logger.info("✓ All essential tools are available")

    def _create_tool_symlinks(self, missing_tools: List[str]):
        """Create symlinks for missing tools if they exist in Homebrew directories."""
        logger.info("Attempting to create symlinks for missing tools...")
        
        homebrew_prefix = '/opt/homebrew' if PlatformDetector.get_arch() == 'arm64' else '/usr/local'
        target_bin_dir = f"{homebrew_prefix}/bin"
        
        tool_locations = {
            'aclocal': f"{homebrew_prefix}/opt/automake/bin/aclocal",
            'automake': f"{homebrew_prefix}/opt/automake/bin/automake",
            'autoconf': f"{homebrew_prefix}/opt/autoconf/bin/autoconf",
            'autoheader': f"{homebrew_prefix}/opt/autoconf/bin/autoheader",
            'autoreconf': f"{homebrew_prefix}/opt/autoconf/bin/autoreconf",
            'autom4te': f"{homebrew_prefix}/opt/autoconf/bin/autom4te",
            'libtool': f"{homebrew_prefix}/opt/libtool/bin/libtool",
            'libtoolize': f"{homebrew_prefix}/opt/libtool/bin/libtoolize",
            'm4': f"{homebrew_prefix}/opt/m4/bin/m4",
        }
        
        for tool in missing_tools:
            if tool in tool_locations:
                source_path = tool_locations[tool]
                target_path = f"{target_bin_dir}/{tool}"
                
                if os.path.exists(source_path) and not os.path.exists(target_path):
                    try:
                        # Create symlink
                        os.symlink(source_path, target_path)
                        logger.info(f"✓ Created symlink for {tool}: {target_path} -> {source_path}")
                    except Exception as e:
                        logger.debug(f"✗ Failed to create symlink for {tool}: {e}")
                elif os.path.exists(source_path):
                    logger.debug(f"Tool {tool} exists at {source_path} but target {target_path} already exists")
                else:
                    logger.debug(f"Tool {tool} not found at expected location {source_path}")
    
    def _diagnose_missing_tools(self, missing_tools: List[str]):
        """Diagnose why tools are missing and suggest solutions."""
        logger.info("Diagnosing missing tools...")
        
        homebrew_prefix = '/opt/homebrew' if PlatformDetector.get_arch() == 'arm64' else '/usr/local'
        
        for tool in missing_tools:
            logger.info(f"Diagnosing {tool}:")
            
            # Check if the tool exists anywhere in Homebrew
            possible_locations = [
                f"{homebrew_prefix}/bin/{tool}",
                f"{homebrew_prefix}/opt/autoconf/bin/{tool}",
                f"{homebrew_prefix}/opt/automake/bin/{tool}",
                f"{homebrew_prefix}/opt/libtool/bin/{tool}",
                f"{homebrew_prefix}/opt/m4/bin/{tool}",
                f"{homebrew_prefix}/opt/gettext/bin/{tool}",
            ]
            
            found_locations = []
            for location in possible_locations:
                if os.path.exists(location):
                    found_locations.append(location)
            
            if found_locations:
                logger.info(f"  Found {tool} at: {', '.join(found_locations)}")
                logger.info(f"  Consider adding these paths to your shell configuration")
            else:
                logger.info(f"  {tool} not found in expected Homebrew locations")
                logger.info(f"  You may need to reinstall the relevant package")

    def reinstall_autotools(self):
        """Reinstall autotools packages to ensure they're properly linked."""
        logger.info("Reinstalling and relinking autotools packages...")
        
        critical_packages = ['autoconf', 'automake', 'libtool', 'm4', 'gettext']
        
        for package in critical_packages:
            try:
                # Uninstall and reinstall to ensure clean state
                logger.info(f"Reinstalling {package}...")
                CommandRunner.run(['brew', 'uninstall', '--ignore-dependencies', package], check=False)
                CommandRunner.run(['brew', 'install', package], check=False)
                CommandRunner.run(['brew', 'link', '--overwrite', package], check=False)
                logger.info(f"✓ Reinstalled and linked {package}")
            except Exception as e:
                logger.warning(f"✗ Failed to reinstall {package}: {e}")
        
        # Force link again
        for package in critical_packages:
            try:
                CommandRunner.run(['brew', 'link', '--force', '--overwrite', package], check=False)
            except:
                pass

    def install_basic_dependencies(self):
        """Install basic build dependencies for macOS."""
        logger.info("Installing basic build dependencies for macOS...")
        
        # Ensure Homebrew is available
        if not self.is_homebrew_available:
            self._install_homebrew()
        
        # Update Homebrew
        try:
            CommandRunner.run(['brew', 'update'])
        except Exception as e:
            logger.warning(f"Failed to update Homebrew: {e}")
        
        # Core development tools
        core_packages = [
            'cmake',
            'ninja', 
            'git',
            'pkg-config'
        ]
        
        # Essential autotools - critical for building many vcpkg packages
        autotools_packages = [
            'autoconf',          # Contains autoconf, autoheader, autom4te, etc.
            'automake',          # Contains aclocal, automake, etc.
            'libtool',           # Contains libtool, libtoolize, etc.
            'autoconf-archive',  # Additional autoconf macros - CRITICAL for ICU
            'm4',                # Macro processor used by autotools
            'gettext',           # Internationalization tools
        ]
        
        # Build tools and utilities
        build_tools = [
            'make',          # GNU make
            'flex',          # Fast lexical analyzer
            'bison',         # Parser generator
            'texinfo',       # Documentation system
            'help2man',      # Generate man pages
            'nasm',          # Assembler
            'yasm',          # Assembler
        ]
        
        # Python development
        python_packages = [
            'python@3.11',   # Python 3.11
            'python@3.12',   # Python 3.12 (backup)
        ]
        
        # Additional utilities
        utility_packages = [
            'wget',
            'curl',
            'rsync',
            'zip',
            'unzip',
            'gnu-tar',       # GNU tar (some builds expect GNU tar)
            'gnu-sed',       # GNU sed (some builds expect GNU sed)
            'coreutils',     # GNU core utilities
            'findutils',     # GNU find, xargs, etc.
            'meson',         # Modern build system
        ]
        
        # SSL/TLS and compression libraries
        library_packages = [
            'openssl@3',
            'zlib',
            'bzip2',
            'xz',
            'lz4',
            'zstd',
        ]
        
        # Install packages in groups
        package_groups = [
            ("Core development tools", core_packages),
            ("Autotools (critical for ICU and other packages)", autotools_packages),
            ("Build tools", build_tools),
            ("Python", python_packages),
            ("Utilities", utility_packages),
            ("Libraries", library_packages)
        ]
        
        all_failed_packages = []
        
        for group_name, packages in package_groups:
            if not packages:
                continue
            
            logger.info(f"Installing {group_name}...")
            failed_packages = self._safe_brew_install(packages)
            
            if failed_packages:
                logger.warning(f"Failed to install from {group_name}: {', '.join(failed_packages)}")
                all_failed_packages.extend(failed_packages)
            else:
                logger.info(f"✓ {group_name} installed successfully")
        
        # Install Python modules
        self._install_python_modules()

        # Critical: Reinstall autotools to ensure proper linking
        self.reinstall_autotools()

        # Link essential tools to ensure they're in PATH
        self._link_essential_tools()

        # --- relink automake aclocal ---
        try:
            CommandRunner.run(['brew', 'unlink', 'automake'], check=False)
            CommandRunner.run(['brew', 'link', '--force', '--overwrite', 'automake'], check=False)
            logger.info("✓ automake relinked")
        except Exception as e:
            logger.warning(f"Failed to relink automake: {e}")

        # check aclocal validity
        aclocal_path = '/usr/local/bin/aclocal' if PlatformDetector.get_arch() != 'arm64' else '/opt/homebrew/bin/aclocal'
        automake_aclocal = '/usr/local/opt/automake/bin/aclocal' if PlatformDetector.get_arch() != 'arm64' else '/opt/homebrew/opt/automake/bin/aclocal'
        if not os.path.exists(aclocal_path) and os.path.exists(automake_aclocal):
            try:
                os.symlink(automake_aclocal, aclocal_path)
                logger.info(f"✓ Created symlink for aclocal: {aclocal_path} -> {automake_aclocal}")
            except Exception as e:
                logger.warning(f"Failed to create symlink for aclocal: {e}")
        elif os.path.exists(aclocal_path):
            logger.info("✓ aclocal is available")
        else:
            logger.warning("aclocal not found, automake may not be installed correctly")

    def _install_python_modules(self):
        """Install Python modules needed for building."""
        logger.info("Installing Python modules for macOS...")
        
        # Try to use pip3 to install essential modules
        python_modules = [
            'jinja2',
            'packaging', 
            'setuptools',
            'wheel',
            'pyyaml',
            'requests',
            'mako',
            'pygments',
            'docutils'
        ]
        
        # Try with pip3 first
        for module in python_modules:
            try:
                if CommandRunner.check_command_exists('pip3'):
                    CommandRunner.run(['pip3', 'install', '--user', module], check=False)
                    logger.debug(f"✓ {module} installed via pip3")
                elif CommandRunner.check_command_exists('python3'):
                    CommandRunner.run(['python3', '-m', 'pip', 'install', '--user', module], check=False)
                    logger.debug(f"✓ {module} installed via python3 -m pip")
            except Exception:
                logger.debug(f"✗ Failed to install {module}")
        
        logger.info("Python module installation completed")

    def install_code_quality_tools(self):
        """Install code quality tools on macOS."""
        logger.info("Installing code quality tools for macOS...")
        
        # Ensure Homebrew is available
        if not self.is_homebrew_available:
            self._install_homebrew()
        
        # Code quality tools
        code_quality_packages = [
            'clang-format',      # Code formatter
            'cppcheck',          # Static analysis tool
            'llvm',              # Contains clang-tidy
        ]
        
        logger.info("Installing code quality packages...")
        failed_packages = self._safe_brew_install(code_quality_packages)
        
        if failed_packages:
            logger.warning(f"Failed to install code quality tools: {', '.join(failed_packages)}")
        else:
            logger.info("✓ Code quality tools installed successfully")
        
        # Install cpplint via pip
        self._install_cpplint()
    
    def _install_cpplint(self):
        """Install cpplint via pip on macOS."""
        logger.info("Installing cpplint via pip...")
        
        try:
            strategies = [
                ['pip3', 'install', '--user', 'cpplint'],
                ['python3', '-m', 'pip', 'install', '--user', 'cpplint'],
            ]
            
            for strategy in strategies:
                try:
                    CommandRunner.run(strategy, check=False)
                    # Verify installation
                    result = CommandRunner.run(['cpplint', '--version'], capture=True, check=False)
                    if result.returncode == 0:
                        logger.info("✓ cpplint installed successfully")
                        return
                except:
                    continue
            
            logger.warning("Could not install cpplint")
                
        except Exception as e:
            logger.warning(f"Failed to install cpplint: {e}")

class WindowsDependencyInstaller:
    """Install dependencies on Windows."""
    
    def __init__(self):
        self.platform = PlatformDetector.get_platform()
        self.arch = PlatformDetector.get_arch()
        self.project_root = Path(__file__).parent.parent.absolute()
        self.env_manager = EnvironmentManager()
        self.checker = DependencyChecker()
        self.code_quality_only = False
        
        if self.platform == 'windows':
            self.installer = WindowsDependencyInstaller()
        elif self.platform == 'macos':
            self.installer = MacOSDependencyInstaller()
        else:
            self.installer = None
    
    def _detect_visual_studio(self):
        """Detect Visual Studio installation."""
        vs_paths = [
            r"C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise",
            r"C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional", 
            r"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community",
            r"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Enterprise",
            r"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional",
            r"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community"
        ]
        
        for vs_path in vs_paths:
            if os.path.exists(vs_path):
                vcvars_path = os.path.join(vs_path, "VC", "Auxiliary", "Build", "vcvarsall.bat")
                if os.path.exists(vcvars_path):
                    logger.info(f"✓ Found Visual Studio at: {vs_path}")
                    return vs_path, vcvars_path
        
        logger.error("✗ Visual Studio 2019/2022 not found")
        return None, None

    def install_basic_dependencies(self):
        """Install basic build dependencies for Windows."""
        logger.info("Installing basic build dependencies for Windows...")
        
        # Detect Visual Studio
        vs_path, vcvars_path = self._detect_visual_studio()
        if not vs_path:
            logger.error("Visual Studio is required for Windows builds")
            return False
        
        # Check if vcpkg is available
        vcpkg_path = os.path.join(os.getcwd(), "tools", "vcpkg", "vcpkg.exe")
        if not os.path.exists(vcpkg_path):
            logger.warning("vcpkg not found, some packages may not be available")
        else:
            logger.info(f"✓ vcpkg found at: {vcpkg_path}")
        
        # Check required tools
        required_tools = ['cmake', 'ninja']
        all_tools_available = True
        
        for tool in required_tools:
            if CommandRunner.check_command_exists(tool):
                logger.info(f"✓ {tool} is available")
            else:
                logger.error(f"✗ {tool} is not available")
                all_tools_available = False
        
        if not all_tools_available:
            logger.error("Some required tools are missing. Please install them manually.")
            return False
        
        logger.info("✓ All basic dependencies are available")
        return True


class VcpkgInstaller:
    """Install and setup vcpkg."""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.vcpkg_root = project_root / "tools" / "vcpkg"
        self.platform = PlatformDetector.get_platform()
    
    def install(self) -> Path:
        """Install vcpkg."""
        tools_dir = self.project_root / "tools"
        tools_dir.mkdir(exist_ok=True)
        
        if self.vcpkg_root.exists():
            logger.info("vcpkg directory exists, updating...")
            try:
                CommandRunner.run(['git', 'pull'], cwd=str(self.vcpkg_root))
            except subprocess.CalledProcessError:
                logger.warning("Failed to update vcpkg, continuing with existing installation")
        else:
            logger.info("Installing vcpkg...")
            CommandRunner.run([
                'git', 'clone', 'https://github.com/Microsoft/vcpkg.git',
                '--depth', '1', str(self.vcpkg_root)
            ])
        
        # Bootstrap vcpkg
        logger.info("Bootstrapping vcpkg...")
        if self.platform == 'windows':
            bootstrap_script = self.vcpkg_root / "bootstrap-vcpkg.bat"
            CommandRunner.run([str(bootstrap_script)], cwd=str(self.vcpkg_root))
        else:
            bootstrap_script = self.vcpkg_root / "bootstrap-vcpkg.sh"
            CommandRunner.run(['bash', str(bootstrap_script)], cwd=str(self.vcpkg_root))
        
        return self.vcpkg_root


class EnvironmentConfigurator:
    """Main environment configuration class."""
    
    def __init__(self):
        self.platform = PlatformDetector.get_platform()
        self.arch = PlatformDetector.get_arch()
        self.project_root = Path(__file__).parent.parent.absolute()
        self.env_manager = EnvironmentManager()
        self.checker = DependencyChecker()
        self.code_quality_only = False
        
        if self.platform == 'linux':
            self.installer = LinuxDependencyInstaller()
        elif self.platform == 'macos':
            self.installer = MacOSDependencyInstaller()
        else:
            self.installer = None
    
    def run(self):
        """Run the complete configuration process."""
        logger.info(f"Starting environment configuration for {self.platform}-{self.arch}")
        logger.info(f"Project root: {self.project_root}")

        if self.code_quality_only:
            logger.info("Code quality mode: only installing code quality tools")
            return self._install_code_quality_tools_only()
        
        if PlatformDetector.is_wsl():
            logger.info("WSL environment detected - some packages may fail, this is normal")
        
        ubuntu_version = PlatformDetector.get_ubuntu_version()
        if ubuntu_version:
            logger.info(f"Ubuntu {ubuntu_version} detected")
        
        try:
            # Check and install basic tools
            self._check_and_install_basic_tools()
            
            # Check and install vcpkg
            self._check_and_install_vcpkg()
            
            # Setup environment variables
            self._setup_environment()
            
            # Verify installation
            self._verify_installation()
            
            logger.info("✓ Environment configuration completed successfully!")
            logger.info("You can now build the project using your preferred build script.")
            
            return True
            
        except Exception as e:
            logger.error(f"Configuration failed: {e}")
            return False
        
    def _install_code_quality_tools_only(self):
        """Install only code quality tools."""
        try:
            if self.installer:
                # Set flag to install basic dependencies needed for code quality tools
                basic_packages = []
                
                if self.platform == 'linux':
                    # Install minimal dependencies for code quality tools
                    basic_packages = ['curl', 'python3', 'python3-pip', 'python3-dev']
                    CommandRunner.run(['sudo', 'apt-get', 'update'])
                    failed_packages = CommandRunner.safe_install_packages(basic_packages)
                    if failed_packages:
                        logger.warning(f"Failed to install basic packages: {', '.join(failed_packages)}")
                
                # Install code quality tools
                self.installer.install_code_quality_tools()
                
                # Verify code quality tools
                self._verify_code_quality_tools()
                
                logger.info("✓ Code quality tools installation completed!")
                return True
            else:
                logger.warning(f"No installer available for platform {self.platform}")
                return False
                
        except Exception as e:
            logger.error(f"Code quality tools installation failed: {e}")
            return False
        
    def _verify_code_quality_tools(self):
        """Verify code quality tools are installed and working."""
        logger.info("Verifying code quality tools...")
        
        tools_to_check = {
            'clang-format': ['clang-format', '--version'],
            'cppcheck': ['cppcheck', '--version'],
            'cpplint': ['cpplint', '--version'],
        }
        
        if self.platform == 'linux':
            tools_to_check.update({
                'clang-tidy': ['clang-tidy', '--version'],
            })
        
        for tool, cmd in tools_to_check.items():
            if CommandRunner.check_command_exists(cmd[0]):
                try:
                    result = CommandRunner.run(cmd, capture=True, check=False)
                    if result.returncode == 0:
                        version = result.stdout.split('\n')[0] if result.stdout else 'unknown'
                        logger.info(f"✓ {tool}: {version}")
                    else:
                        logger.warning(f"✗ {tool}: version check failed")
                except:
                    logger.warning(f"✗ {tool}: version check failed")
            else:
                logger.warning(f"✗ {tool}: not found")
    
    def _check_and_install_basic_tools(self):
        """Check and install basic development tools."""
        logger.info("Checking basic development tools...")
        
        tools = self.checker.check_basic_tools()
        missing_tools = [tool for tool, installed in tools.items() if not installed]
        
        if missing_tools:
            logger.info(f"Missing tools: {', '.join(missing_tools)}")
            if self.installer:
                self.installer.install_basic_dependencies()
            else:
                logger.warning(f"Please install missing tools manually on {self.platform}")
        else:
            logger.info("✓ All basic development tools are available")
    
    def _check_and_install_vcpkg(self):
        """Check and install vcpkg."""
        logger.info("Checking vcpkg...")
        
        if self.checker.check_vcpkg(self.project_root):
            logger.info("✓ vcpkg is already installed and working")
        else:
            logger.info("Installing vcpkg...")
            vcpkg_installer = VcpkgInstaller(self.project_root)
            vcpkg_root = vcpkg_installer.install()
            
            # Setup environment variables for vcpkg
            self.env_manager.add_env_var('VCPKG_ROOT', str(vcpkg_root), "vcpkg package manager root")
            self.env_manager.add_to_path(str(vcpkg_root), "vcpkg executable directory")
            
            # Set macOS-specific vcpkg environment variables
            if self.platform == 'macos':
                self.env_manager.add_env_var('VCPKG_DEFAULT_TRIPLET', f'{self.arch}-osx', "vcpkg default triplet for macOS")
            
            logger.info(f"✓ vcpkg installed to: {vcpkg_root}")
    
    def _setup_environment(self):
        """Setup environment variables."""
        logger.info("Setting up environment variables...")
        
        # Try to reload environment
        if self.env_manager.reload_environment():
            logger.info("✓ Environment variables are now active")
        else:
            logger.info("Environment variables set for future sessions")
    
    def _verify_installation(self):
        """Verify installed tools."""
        logger.info("Verifying installation...")
        
        tools_to_check = {
            'cmake': ['cmake', '--version'],
            'ninja': ['ninja', '--version'],
            'git': ['git', '--version'],
            'python3': ['python3', '--version'],
        }
        
        if self.platform == 'linux':
            tools_to_check.update({
                'gcc': ['gcc', '--version'],
                'g++': ['g++', '--version'],
                'pkg-config': ['pkg-config', '--version'],
                'autoconf': ['autoconf', '--version'],
                'automake': ['automake', '--version'],
                'libtool': ['libtool', '--version'],
                'meson': ['meson', '--version'],
            })
        elif self.platform == 'macos':
            tools_to_check.update({
                'clang': ['clang', '--version'],
                'clang++': ['clang++', '--version'],
                'pkg-config': ['pkg-config', '--version'],
                'autoconf': ['autoconf', '--version'],
                'automake': ['automake', '--version'],
                'aclocal': ['aclocal', '--version'],  # Specifically check for aclocal
                'libtool': ['libtool', '--version'],
                'autoreconf': ['autoreconf', '--version'],  # Check autoreconf too
                'make': ['make', '--version'],
                'meson': ['meson', '--version'],
            })
        
        for tool, cmd in tools_to_check.items():
            if CommandRunner.check_command_exists(cmd[0]):
                try:
                    result = CommandRunner.run(cmd, capture=True, check=False)
                    if result.returncode == 0:
                        version = result.stdout.split('\n')[0] if result.stdout else 'unknown'
                        logger.info(f"✓ {tool}: {version}")
                    else:
                        logger.warning(f"✗ {tool}: version check failed")
               
                except:
                    logger.warning(f"✗ {tool}: version check failed")
            else:
                logger.warning(f"✗ {tool}: not found")
        
        # Check Python modules
        if self.platform in ['linux', 'macos']:
            self._verify_python_modules()
        
        # Check vcpkg
        if self.checker.check_vcpkg(self.project_root):
            vcpkg_path = self.project_root / 'tools' / 'vcpkg' / ('vcpkg.exe' if self.platform == 'windows' else 'vcpkg')
            try:
                result = CommandRunner.run([str(vcpkg_path), 'version'], capture=True, check=False)
                if result.returncode == 0:
                    version = result.stdout.split('\n')[0] if result.stdout else 'unknown'
                    logger.info(f"✓ vcpkg: {version}")
            except:
                logger.info("✓ vcpkg: installed")
    
    def _verify_python_modules(self):
        """Verify Python modules are available."""
        important_modules = ['jinja2', 'packaging', 'setuptools', 'wheel', 'mako']
        
        logger.info("Checking Python modules...")
        for module in important_modules:
            try:
                result = CommandRunner.run(
                    ['python3', '-c', f'import {module}; print({module}.__version__)'],
                    capture=True, check=False
                )
                if result.returncode == 0:
                    version = result.stdout.strip() if result.stdout else 'unknown'
                    logger.info(f"✓ python3-{module}: {version}")
                else:
                    logger.warning(f"✗ python3-{module}: not available")
            except:
                logger.warning(f"✗ python3-{module}: not available")


def main():
    """Main entry point."""
    import argparse
    
    parser = argparse.ArgumentParser(description="Configure development environment")
    parser.add_argument("--platform", choices=["linux", "macos", "windows"], 
                       help="Force platform detection")
    parser.add_argument("--no-deps", action="store_true", 
                       help="Skip system dependency installation")
    parser.add_argument("--no-vcpkg", action="store_true", 
                       help="Skip vcpkg setup")
    parser.add_argument("--no-python-modules", action="store_true",
                       help="Skip Python module installation")
    parser.add_argument("--code-quality", action="store_true",
                       help="Install only code quality tools (clang-format, cppcheck, cpplint)")
    parser.add_argument("--verbose", "-v", action="store_true", 
                       help="Verbose output")
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    platform_info = f"{PlatformDetector.get_platform()}-{PlatformDetector.get_arch()}"
    logger.info(f"Detected platform: {platform_info}")
    
    if PlatformDetector.is_wsl():
        logger.info("WSL detected - will handle package installation carefully")
    
    configurator = EnvironmentConfigurator()
    
    # Set code quality mode
    if args.code_quality:
        configurator.code_quality_only = True
    
    # Pass options to configurator
    if hasattr(configurator, 'installer') and configurator.installer:
        if hasattr(configurator.installer, 'skip_python_modules'):
            configurator.installer.skip_python_modules = args.no_python_modules
    
    success = configurator.run()
    
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()