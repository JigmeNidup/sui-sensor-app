"use client"

import { QueryClient, QueryClientProvider } from "@tanstack/react-query"
import { SuiClientProvider, WalletProvider, createNetworkConfig } from "@mysten/dapp-kit"
import { getFullnodeUrl } from "@mysten/sui/client"
import "@mysten/dapp-kit/dist/index.css"
import { type ReactNode } from "react"

const { networkConfig } = createNetworkConfig({
  testnet: { url: getFullnodeUrl("testnet") },
  mainnet: { url: getFullnodeUrl("mainnet") },
})

const queryClient = new QueryClient()

export default function DappKitProvider({ children }: { children: ReactNode }) {
  return (
    <QueryClientProvider client={queryClient}>
      <SuiClientProvider networks={networkConfig} defaultNetwork="testnet">
        <WalletProvider autoConnect>
          {children}
        </WalletProvider>
      </SuiClientProvider>
    </QueryClientProvider>
  )
}