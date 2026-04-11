# Aura Retail OS – Subtask 2

Partial implementation of the Aura Retail OS kiosk system.
Covers two design patterns: **Adapter** (payment) and **Strategy** (pricing).

---

## What's implemented

- `Item` / `Product` – basic inventory item
- `Inventory` – add products, reduce stock on purchase
- `Payment` + `CardAdapter`, `UPIAdapter`, `WalletAdapter` – three payment methods via Adapter pattern
- `PricingPolicy` + `StandardPricing`, `DiscountPricing` – two pricing strategies via Strategy pattern
- `main.cpp` – runs a small purchase simulation

Not included in this subtask: Kiosk core, Factory, Command, Singleton, Composite, Proxy, persistence layer.

---

## Folder structure

```
AuraRetailOS-Subtask2/
├── include/
│   ├── inventory/   (Item.h, Product.h, Inventory.h)
│   ├── payment/     (Payment.h, CardAdapter.h, UPIAdapter.h, WalletAdapter.h)
│   └── pricing/     (PricingPolicy.h, StandardPricing.h, DiscountPricing.h)
└── src/
    ├── inventory/   (Product.cpp, Inventory.cpp)
    ├── payment/     (CardAdapter.cpp, UPIAdapter.cpp, WalletAdapter.cpp)
    ├── pricing/     (StandardPricing.cpp, DiscountPricing.cpp)
    └── main.cpp
```

---

## How to compile

```bash
g++ -std=c++17 -Iinclude \
  src/inventory/Product.cpp \
  src/inventory/Inventory.cpp \
  src/payment/CardAdapter.cpp \
  src/payment/UPIAdapter.cpp \
  src/payment/WalletAdapter.cpp \
  src/pricing/StandardPricing.cpp \
  src/pricing/DiscountPricing.cpp \
  src/main.cpp -o AuraRetailOS
```

Then run:
```bash
./AuraRetailOS        # Linux/Mac
AuraRetailOS.exe      # Windows
```

---

## Design patterns used

### Adapter – Payment system
The kiosk talks to all payment methods through one interface (`Payment`).
Each adapter wraps a different external SDK and translates the `pay()` call into whatever that SDK expects — `chargeCard()` for card, `initiateUPITransaction()` for UPI, `debitWallet()` for wallet.
Swapping payment methods doesn't change anything else in the code.

### Strategy – Pricing system
`PricingPolicy` is the strategy interface. `StandardPricing` returns the base price as-is. `DiscountPricing` applies a percentage discount.
The purchase function just calls `computePrice(item)` — it doesn't care which strategy is active.
In the full system, the discount rate would come from `CentralRegistry`; here it's passed directly.

---

## Sample output

```
>> Inventory Setup
  [+] Mineral Water   | Price: Rs.20.00  | Stock: 5
  [+] Chocolate Bar   | Price: Rs.50.00  | Stock: 3
  [+] Aspirin Pack    | Price: Rs.120.00 | Stock: 2

>> Purchase #1 - Mineral Water
  [STRATEGY] StandardPricing  -> Rs.20.00
  [ADAPTER]  CardAdapter      -> chargeCard(Rs.20.00) -> APPROVED
  [OK] Purchase successful | Remaining stock: 4
```

---
