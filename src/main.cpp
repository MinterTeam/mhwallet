/*!
 * mhwallet.
 * main.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "minter/nanos_wallet.h"

#include <boost/program_options.hpp>
#include <exception>
#include <functional>
#include <iostream>
#include <unordered_map>

namespace po = boost::program_options;

typedef int (*action_func_t)(const boost::program_options::variables_map&);
typedef void (*action_options_func_t)(boost::program_options::options_description&);

// get address
static void action_get_address_opts(boost::program_options::options_description& options) {
    // clang-format off
    options.add_options()
        ("index,i", po::value<uint32_t>()->default_value(0), "BIP32 derivation index")
        ("silent,s", "Don't show comparison text, just return address")
        ("inline", "Print result inline without newlines etc");
    // clang-format on
}
static int action_get_address(const boost::program_options::variables_map& options) {
    minter::nanos_wallet device;
    if (!device.init()) {
        return 1;
    }

    uint32_t derivationIndex = options.at("index").as<uint32_t>();
    bool printInline = options.count("inline");
    bool silent = options.count("silent");
    auto address = device.get_address(derivationIndex, silent);

    if (printInline) {
        std::cout << address.to_string();
        return 0;
    }

    std::cout << "Address: " << address.to_string() << "\n";
    return 0;
}

// version
static void action_app_version_opts(boost::program_options::options_description& options) {
    options.add_options()("inline", "Print result inline without newlines etc");
}
static int action_app_version(const boost::program_options::variables_map& options) {
    minter::nanos_wallet device;
    if (!device.init()) {
        return 1;
    }

    bool printInline = options.count("inline");

    auto vers = device.get_app_version();
    if (printInline) {
        std::cout << vers;
        return 0;
    }

    std::cout << "Version: " << vers << std::endl;
    return 0;
}

// sign
static void action_sign_tx_opts(boost::program_options::options_description& options) {
    options.add_options()("hash", po::value<std::string>()->required(), "Incomplete transaction sha3 hash (64 byte hex string)")("index,i", po::value<uint32_t>()->default_value(0), "BIP32 derivation index")("inline", "Print result inline without newlines etc");
}
static int action_sign_tx(const boost::program_options::variables_map& options) {
    //    std::string txHash = "067ea2037c3f14e7fb1affaec93940bc5af500144f13274e131c9ae78cbbd4e1"; // this has recovery 0x1c
    //    std::string txHash = "1ee24f115b579f0f1ba7278515f8c438c2da201dc37fa44c2d9f431d94a9693e"; // this has recovery 0x1b
    //    std::string txHash = "18b91c804a8aba5e5d1f4acd83b60b74925afda8ce06b26e03575ad2f89f4bef"; // this has overflow in R component and leading zero
    minter::nanos_wallet device;
    if (!device.init()) {
        return 1;
    }

    tb::bytes_data txHash = options.at("hash").as<std::string>();
    uint32_t derivationIndex = options.at("index").as<uint32_t>();
    bool printInline = options.count("inline");

    auto address = device.get_address(0, true);
    auto sign = device.sign_tx(txHash, derivationIndex);

    tb::bytes_data outInline(65);
    outInline.write(0, sign.r);
    outInline.write(32, sign.s);
    outInline.write(64, sign.v);

    if (printInline) {
        std::cout << outInline.to_hex();
        return 0;
    }
    std::cout << "Signature result:\n";
    std::cout << "R:   " << minter::dumpHexRet(sign.r) << "\n";
    std::cout << "S:   " << minter::dumpHexRet(sign.s) << "\n";
    std::cout << "V:   " << minter::dumpHexRet(sign.v) << "\n\n";
    std::cout << "RSV: " << outInline.to_hex() << std::endl;

    return 0;
}

const static std::unordered_map<std::string, action_func_t> action_map = {
    {"get-address", &action_get_address},
    {"sign-tx", &action_sign_tx},
    {"app-version", &action_app_version},
};

const static std::unordered_map<std::string, action_options_func_t> opts_map = {
    {"get-address", &action_get_address_opts},
    {"sign-tx", &action_sign_tx_opts},
    {"app-version", &action_app_version_opts},
};

int main(int argc, char** argv) {
    std::string actions_desc = R"(Minter Ledger Wallet CLI

Usage:
 Command list:
   get-address - Resolve minter address from device

   sign-tx     - Sign passed transaction hash (must contains exact 64 byte hex string)

   app-version - Return Minter ledger application version

Options)";

    po::options_description desc(actions_desc);
    desc.add_options()("help,h", "Prints this help or use it with command name to see additional parameters")("version,v", "print version of client")("verbose", "print data transfer process");

    po::variables_map vm;

    auto handleArgs = [&vm, &argc, &argv, &desc] {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return true;
        }

        HIDPP_VERBOSE = true;

        try {
            po::notify(vm);
        } catch (const std::exception& e) {
            std::cerr << "Error:\n  ";
            std::cerr << e.what() << "\n"
                      << std::endl;
            std::cout << desc << std::endl;
            return false;
        }

        return true;
    };

    if (argc == 1) {
        const auto ret = handleArgs();
        std::cout << desc << std::endl;
        return ret ? 0 : 1;
    }

    if (argc >= 2) {
        std::string cmdName = argv[1];

        if (!action_map.count(cmdName)) {
            std::cerr << "Unknown command: " << cmdName << "\n\n";
            std::cout << desc << std::endl;
            return 1;
        }

        opts_map.at(cmdName)(desc);
        if (!handleArgs()) {
            return 1;
        }
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        try {
            return action_map.at(cmdName)(vm);
        } catch (const std::exception& e) {
            ML_ERR(e.what());
            return 1;
        }
    }

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    return 0;
}
