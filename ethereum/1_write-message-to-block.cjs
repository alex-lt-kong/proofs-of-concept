require('dotenv').config();
const { ethers } = require('ethers');
const readline = require('readline');

async function askUser(prompt) {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
    });
    return new Promise(resolve => rl.question(prompt, ans => {
        rl.close();
        resolve(ans.trim().toLowerCase());
    }));
}

function getProvider(network) {
    const infuraUrl = `https://${network}.infura.io/v3/${process.env.INFURA_PROJECT_ID}`;
    return new ethers.JsonRpcProvider(infuraUrl);
}

async function sendTestMessage() {
    const network = process.env.NETWORK || 'mainnet';
    const provider = getProvider(network);
    const wallet = new ethers.Wallet(process.env.PRIVATE_KEY, provider);

    const message = `Hello Ethereum ${network}!`;
    const tx = {
        to: wallet.address,
        value: ethers.parseEther("0.00001"),
        data: ethers.hexlify(ethers.toUtf8Bytes(message)),
    };

    // Estimate gas
    const estimatedGas = await provider.estimateGas({ ...tx, from: wallet.address });
    // const gasPrice = await provider.getGasPrice();
    const feeData = await provider.getFeeData();
    const gasPrice = feeData.gasPrice;
    const gasCost = gasPrice * estimatedGas;
    const gasCostEth = ethers.formatEther(gasCost);

    console.log(`🌐 Network: ${network}`);
    console.log(`📊 Estimated Gas: ${estimatedGas.toString()} units`);
    console.log(`⛽ Gas Price: ${ethers.formatUnits(gasPrice, "gwei")} gwei`);
    console.log(`💰 Estimated Fee: ${gasCostEth} ETH`);

    const confirm = await askUser("Proceed with transaction? (yes/no): ");
    if (confirm !== 'yes') {
        console.log("❌ Transaction cancelled.");
        return;
    }

    tx.gasLimit = estimatedGas;

    const response = await wallet.sendTransaction(tx);
    console.log("✅ Transaction sent! Hash:", response.hash);

    const receipt = await response.wait();
    console.log("📦 Confirmed in block:", receipt.blockNumber);
}

sendTestMessage().catch(console.error);
