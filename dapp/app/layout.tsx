import type { Metadata } from "next"
import { Inter } from "next/font/google"
import "./globals.css"
import DappKitProvider from "@/components/DappKitProvider"

const inter = Inter({ subsets: ["latin"] })

export const metadata: Metadata = {
  title: "Sui Sensor Data Storage",
  description: "Store and visualize IoT sensor data on the Sui blockchain",
}

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode
}>) {
  return (
    <html lang="en">
      <body className={`${inter.className} bg-linear-to-br from-gray-900 via-gray-800 to-gray-900`}>
        <DappKitProvider>
          <div className="min-h-screen">
            {/* Navigation Bar */}
            <nav className="sticky top-0 z-50 border-b border-gray-700/50 bg-gray-900/80 backdrop-blur-md">
              <div className="mx-auto px-4 py-4">
                <div className="flex items-center justify-between">
                  <div className="flex items-center space-x-4">
                    <div className="h-8 w-8 rounded-full bg-linear-to-r from-green-400 to-blue-500 flex items-center justify-center">
                      <svg className="h-5 w-5 text-white" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                        <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z" />
                      </svg>
                    </div>
                    <div>
                      <h1 className="text-xl font-bold bg-linear-to-r from-green-400 to-blue-500 bg-clip-text text-transparent">
                        Sui Sensor Network
                      </h1>
                      <p className="text-xs text-gray-400">Decentralized IoT Data Platform</p>
                    </div>
                  </div>
                  
                  <div className="hidden md:flex items-center space-x-6">
                    <a href="#" className="text-gray-300 hover:text-white transition-colors text-sm">Dashboard</a>
                    <a href="#" className="text-gray-300 hover:text-white transition-colors text-sm">Analytics</a>
                    <a href="#" className="text-gray-300 hover:text-white transition-colors text-sm">Devices</a>
                    <a href="#" className="text-gray-300 hover:text-white transition-colors text-sm">Documentation</a>
                  </div>
                </div>
              </div>
            </nav>

            {/* Main Content */}
            <main className="mx-auto px-4 py-8">
              {children}
            </main>

            {/* Footer */}
            <footer className="border-t border-gray-700/50 bg-gray-900/50 mt-12">
              <div className="container mx-auto px-4 py-8">
                <div className="grid grid-cols-1 md:grid-cols-3 gap-8">
                  <div>
                    <h3 className="text-lg font-semibold text-white mb-4">About</h3>
                    <p className="text-gray-400 text-sm">
                      A decentralized platform for storing and analyzing IoT sensor data on the Sui blockchain.
                      All data is immutable, verifiable, and owned by you.
                    </p>
                  </div>
                  
                  <div>
                    <h3 className="text-lg font-semibold text-white mb-4">Resources</h3>
                    <ul className="space-y-2 text-sm text-gray-400">
                      <li>
                        <a href="https://docs.sui.io/" target="_blank" rel="noopener noreferrer" className="hover:text-white transition-colors">
                          Sui Documentation
                        </a>
                      </li>
                      <li>
                        <a href="https://github.com/MystenLabs/sui" target="_blank" rel="noopener noreferrer" className="hover:text-white transition-colors">
                          GitHub Repository
                        </a>
                      </li>
                      <li>
                        <a href="https://suiscan.xyz/testnet" target="_blank" rel="noopener noreferrer" className="hover:text-white transition-colors">
                          Block Explorer
                        </a>
                      </li>
                      <li>
                        <a href="https://docs.sui.io/guides/developer/getting-started/sui-faucet" target="_blank" rel="noopener noreferrer" className="hover:text-white transition-colors">
                          Testnet Faucet
                        </a>
                      </li>
                    </ul>
                  </div>
                  
                  <div>
                    <h3 className="text-lg font-semibold text-white mb-4">Network Status</h3>
                    <div className="flex items-center space-x-2">
                      <div className="h-2 w-2 rounded-full bg-green-500 animate-pulse"></div>
                      <span className="text-sm text-gray-300">Testnet: Operational</span>
                    </div>
                    <p className="text-xs text-gray-500 mt-2">
                      Connected to Sui Testnet. Switch networks in your wallet.
                    </p>
                  </div>
                </div>
                
                <div className="mt-8 pt-8 border-t border-gray-700/50 text-center">
                  <p className="text-sm text-gray-500">
                    © {new Date().getFullYear()} Sui Sensor Network. Built with Sui Move and Next.js.
                  </p>
                </div>
              </div>
            </footer>
          </div>
        </DappKitProvider>
      </body>
    </html>
  )
}