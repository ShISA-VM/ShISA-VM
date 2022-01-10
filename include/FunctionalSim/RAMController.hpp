#pragma once

#include <ShISA/Binary.hpp>
#include <ShISA/ISAModule.hpp>

#include <array>
#include <climits>
#include <concepts>
#include <iomanip>
#include <iostream>
#include <limits>
#include <ostream>



namespace shisa::fsim {

template <typename addr_t, typename cell_t>
requires(std::unsigned_integral<addr_t>
             &&std::unsigned_integral<cell_t>) class RAMBase {
public:
  using Addr       = addr_t;
  using Cell       = cell_t;
  using MemStorage = std::array<Cell, std::numeric_limits<Addr>::max() + 1>;

  static_assert(std::numeric_limits<Addr>::is_integer &&
                    !std::numeric_limits<Addr>::is_signed,
                "addr_t must be an unsigned integer type");
  static_assert(std::numeric_limits<Cell>::is_integer &&
                    !std::numeric_limits<Cell>::is_signed,
                "cell_t must be an unsigned integer type");

private:
  MemStorage storage{};

public:
  void dump(std::ostream &os) const {
    os << "RAM dump\n";
    Addr addr = 0;
    for (const auto cell : storage) {
      os << "0x" << std::setw(2 * sizeof(Addr)) << std::right << std::hex
         << std::setfill('0') << addr++ << " = 0x"
         << std::setw(2 * sizeof(Cell)) << std::right << std::hex
         << std::setfill('0') << static_cast<unsigned>(cell) << "\n";
    }
  }

  auto begin() { return storage.begin(); }
  auto end() { return storage.end(); }

  auto begin() const { return storage.begin(); }
  auto end() const { return storage.end(); }

  auto read(Addr addr) const -> Cell { return storage[addr]; }
  void write(Addr addr, Cell data) { storage[addr] = data; }
};



template <typename addr_t, typename cell_t>
requires(std::unsigned_integral<addr_t>
             &&std::unsigned_integral<cell_t>) class RAMControllerBase {
public:
  using Addr = addr_t;
  using Cell = cell_t;

  static constexpr bool instEndAligned =
      (sizeof(ISAModule::RawInst) % sizeof(Cell)) == 0;
  static constexpr size_t cellsPerInst =
      sizeof(ISAModule::RawInst) / sizeof(Cell) + (instEndAligned ? 0 : 1);

  static constexpr bool dataEndAligned =
      (sizeof(Binary::Data) % sizeof(Cell)) == 0;
  static constexpr size_t cellsPerData =
      sizeof(Binary::Data) / sizeof(Cell) + (dataEndAligned ? 0 : 1);

private:
  using RAM = RAMBase<Addr, Cell>;
  RAM ram{};

  bool binaryLoaded = false;

  addr_t dataEnd   = 0x0000;
  addr_t binaryEnd = 0x0000;

public:
  using MemStorage = typename RAM::MemStorage;

  auto begin() { return ram.begin(); }
  auto end() { return ram.end(); }

  auto begin() const { return ram.begin(); }
  auto end() const { return ram.end(); }

  void dump(std::ostream &os) const {
    os << "RAMController dump\n";
    ram.dump(os);
  }

  void loadBin(const Binary &b) {
    addr_t currAddr = 0;

    for (const auto data : b.getRawData()) {
      for (int i = cellsPerData - 1; i >= 0; i--) {
        ram.write(currAddr++, (data >> (i * sizeof(Cell) * CHAR_BIT)) &
                                  std::numeric_limits<cell_t>::max());
      }
    }

    dataEnd = currAddr;

    for (const auto inst : b.getISAModule()) {
      for (int i = cellsPerInst - 1; i >= 0; i--) {
        ram.write(currAddr++, (inst >> (i * sizeof(Cell) * CHAR_BIT)) &
                                  std::numeric_limits<cell_t>::max());
      }
    }

    binaryEnd    = currAddr;
    binaryLoaded = true;
  }

  [[nodiscard]] auto getProgramStart() const -> Addr { return dataEnd; }

  [[nodiscard]] auto getProgramEnd() const -> Addr { return binaryEnd; }

  [[nodiscard]] auto getBinEnd() const -> Addr { return binaryEnd; }

  [[nodiscard]] auto getBinDataAddr() const -> Addr { return 0x0000; }

  [[nodiscard]] auto isBinaryLoaded() const -> bool { return binaryLoaded; }

  auto read(addr_t addr) const -> cell_t { return ram.read(addr); }

  void write(addr_t addr, cell_t data) {
    // binary is loaded in read-only memory
    if (addr >= binaryEnd) {
      ram.write(addr, data);
    }
  }
};

} // namespace shisa::fsim
