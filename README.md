# Aura Retail OS - Modular Kiosk System

Aura Retail OS is a modular kiosk management system designed for versatile retail environments including food, pharmacy, and emergency supply dispensing. The system utilizes multiple software design patterns to ensure scalability, maintainability, and hardware independence.

## Core Features

- Modular Hardware Support: Supports optional modules like Solar Charging and Network connectivity using the Decorator pattern.
- Multi-Role Interface: Dedicated portals for Customers (purchase, wallet management) and Admins (inventory, diagnostics, logs).
- Dynamic Inventory: Real-time stock tracking with Proxy-based monitoring for low-stock alerts.
- Integrated Payment System: Unified payment interface supporting Card, UPI, and Secure Wallet via the Adapter pattern.
- System Diagnosis: Backend-driven diagnostic reporting for hardware, payment, and inventory status.
- Transaction Integrity: Command pattern implementation for purchases with Memento-based rollback on hardware failure.

## Design Patterns Implemented

- Factory Pattern: Used for creating kiosk-specific components (Dispensers, Payments, Inventory) based on the kiosk type (Food, Pharmacy, Emergency).
- Builder Pattern: Facilitates the step-by-step construction of complex Kiosk objects with various optional modules.
- Decorator Pattern: Dynamically adds hardware capabilities (SolarModule, NetworkModule) to the kiosk without altering its core structure.
- Command Pattern: Encapsulates actions like Purchase, Restock, and Refund as objects for audit logging and undo capabilities.
- Strategy Pattern: Implements flexible pricing policies (Standard, Discount, Emergency) that can be swapped at runtime.
- Adapter Pattern: Bridges different payment SDKs (Card, UPI, Wallet) into a single unified Payment interface.
- Proxy Pattern: Manages inventory access with an added layer for logging and real-time stock monitoring.
- Singleton Pattern: Ensures a single instance of the UserWallet and CentralRegistry across the system.
- Memento Pattern: Captures inventory state before transactions to allow safe rollback if hardware dispensing fails.

## Project Structure

- include/: Header files organized by pattern and component.
- src/: Source code implementations.
- gui/: Qt-based desktop application source files and UI definitions.
- data/: Persistent storage for configuration, inventory, users, and transactions.

## Build Instructions

### Prerequisites
- C++17 compatible compiler (MinGW, GCC, or MSVC)
- Qt 6.x Framework (for GUI build)
- CMake 3.16 or higher

### CLI Version
To build the command-line interface:
mkdir build
cd build
cmake ..
cmake --build . --target AuraRetailOsCli

### GUI Version
To build the Qt desktop application:
1. Open the project in Qt Creator.
2. Select the CMakeLists.txt file.
3. Configure the project with a Qt 6.x kit.
4. Build and run the AuraRetailOsGui target.

## Usage

### Admin Portal
- Login with any ID in the Admin role.
- Perform system diagnostics to check hardware and battery status.
- Restock items and view transaction history.
- Simulate hardware states (Battery level, Network connectivity).

### Customer Portal
- Browse available products based on kiosk type.
- Manage wallet balance and perform top-ups.
- Purchase items using multiple payment methods.
- Request refunds using valid Transaction IDs.

## Data Persistence
The system uses JSON for configuration and user data, while transactions are recorded in a CSV format for auditing purposes. All data files are located in the data/ directory.
