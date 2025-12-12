import { NextRequest, NextResponse } from 'next/server';
import { submitSignedTransaction } from '@/lib/transaction-builder';

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    
    // Validate required fields
    if (!body.txBytes || !body.signature) {
      return NextResponse.json(
        {
          success: false,
          error: 'Missing txBytes or signature in request'
        },
        { status: 400 }
      );
    }

    // Submit to Sui
    const result = await submitSignedTransaction(body.txBytes, body.signature);

    return NextResponse.json({
      success: true,
      message: 'Transaction submitted successfully',
      digest: result.digest,
      explorerUrl: `https://suiscan.xyz/testnet/tx/${result.digest}`,
      effects: result.effects,
      timestamp: new Date().toISOString()
    });

  } catch (error: any) {
    console.error('Submit TX Error:', error);
    
    return NextResponse.json(
      {
        success: false,
        error: error.message || 'Failed to submit transaction'
      },
      { status: 500 }
    );
  }
}

export async function GET() {
  return NextResponse.json({
    status: 'active',
    service: 'Sui Transaction Submitter API',
    endpoint: 'POST /api/submit-tx with signed transaction',
    timestamp: new Date().toISOString()
  });
}