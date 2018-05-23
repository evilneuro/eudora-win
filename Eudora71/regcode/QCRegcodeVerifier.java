public class QCRegcodeVerifier
{
	private final int 	REG_CODE_V1 = 0, 
						REG_CODE_V2 = 1, 
						REG_CODE_BAD = 2,
						REG_CODE_LEN = 20;

	private final long 	XOR_1 = 0x01C5E73A, 
						XOR_2 = 0x01F6433E, 
						XOR_NAME = 0x35,
						REG_CODE_SCHEME_VERSION = 0;

	// change internally to byte type because working with String opers is PITA.
	private byte[] szCode, szName, szPureCode;
	private int reglen, namlen;

	private int month = 0, policy = 0;

	// helper class
	private class RegData
	{
		int month;
		int policy;
	}

	public QCRegcodeVerifier(String regName, String regCode)
	throws Exception
	{
		if (regCode == null) throw new Exception("regcode == null");
		if (regName == null) regName = new String("");

		reglen = regCode.length();
		szCode = regCode.getBytes();

		namlen = regName.length();
		szName = regName.getBytes();

		szPureCode = new byte[REG_CODE_LEN];
	}

	private long XORMD4Hash(byte[] str, int len, long nXor)
	{
		int i;
        long lSum = 0, kKRHashPrime = 2147483629;

		for (i = 0; i < len; i++)
		{
			for (int nBit = 0x80; nBit != 0; nBit >>= 1)
			{
				lSum += lSum;
				if (lSum >= kKRHashPrime)
					lSum -= kKRHashPrime;
				if (((str[i] ^ nXor) & nBit) != 0 )
					++lSum;

				if (lSum >= kKRHashPrime)
					lSum -= kKRHashPrime;
			}
		}

		return (lSum+1);
	}

	private boolean CharInStr(int c, String str)
	{
		boolean nReturn = false;

		for(int i = 0; i < str.length(); i++)
		{
			if(str.charAt(i) == c)
			{
				nReturn = true;
				break;
			}
		}

  		return(nReturn);
	}

	private long B12Decode(byte[] str)
	{
		long c;
		long ret = 0;

		for (int i=0; i < 8; i++)
		{
			if (str[i] != '-')
			{
				c = str[i] - '0';
				if (c > 9)
					c = c + '0' - 'A' + 10;
				ret = (ret*12) + (11 - c);
			}
		} 

		return ret;
	}

	private long B10Decode(byte[] str)
	{
		long c;
		long ret = 0;

		for (int i=0; i < 9; i++)
		{
			if (str[i] != '-')
			{
				c = str[i] - '0';
				ret = (ret*10) + (9 - c);
			}
		}

		return ret;
	}

	private long TwoBitCrc(long v)
	{
		long nBit = 0x8000, nCrc = 0;

		while (nBit != 0)
		{
			if ((v & nBit) != 0) nCrc++;
			nBit >>= 1;
		}

		return (nCrc & 0x3);
	}

	private long OneBitCrc(long v)
	{
		long nBit = 0x10, nCrc = 0;

		while (nBit != 0)
		{
			if ((v & nBit) != 0) nCrc++;
			nBit >>= 1;
		}

		return (nCrc & 0x1);
	}

	private long FourBitCrc(long v)
	{
		long nCrc = 0;

		while (v != 0)
		{
			nCrc = (nCrc << 1) + OneBitCrc(v & 0x1f);
			v >>= 5;
		}

		return (nCrc);
	}

	private long SwapBits(long uBits, long uAmount, long nMax)
	{
		long uMask1, uMask2;

		if (uAmount != 0)
		{
			uMask1 = 0x3f >> (6 - uAmount);
			uMask2 = uMask1 << uAmount;

			while (uMask2 < (1 << nMax))
			{
				long uTmp;

				uTmp = (uBits & uMask1) << uAmount;
				uBits = (uBits & ~uMask1) | ((uBits & uMask2) >> uAmount);
				uBits = (uBits & ~uMask2) | uTmp;
				uMask1 <<= uAmount*((long)2);
				uMask2 <<= uAmount*((long)2);
			}
		}

		return uBits;
	}

	private int RegCodeV3Canonicalize(byte[] s, byte[] d)
	{
		int nCharPlusDigitCnt = 0, nReturn, nMod, nCharCount;
		boolean nIsValid;

		for (int i=0; i < reglen; i++)
			if (CharInStr(s[i], "0123456789IOabAB") == true)
				nCharPlusDigitCnt++;

		if ((nCharPlusDigitCnt != 14) && (nCharPlusDigitCnt != 16))
		{
			nReturn = REG_CODE_BAD;
		}
		else
		{
			nReturn = REG_CODE_V2;
			nMod = 4;

			if (nCharPlusDigitCnt == 14)
			{
				nMod = 5;
				nReturn = REG_CODE_V1;
			}

			nCharCount = 0;
			int i = 0, j = 0;

			while (true)
			{
				nIsValid = false;

				if ((s[i] >= '0' && s[i] <= '9') || 
					(s[i] >= 'a' && s[i] <= 'b') ||
					(s[i] >= 'A' && s[i] <= 'B'))
				{
					d[j] = s[i];
					if (d[j] >= 'a')
						d[j] &= ~0x20;
					j++;
					nIsValid = true;
				}
				else if (s[i] == 'O') 
				{
					d[j++] = (byte)'0';
					nIsValid = true;
				}
				else if (s[i] == 'I')
				{
					d[j++] = (byte)'1';
					nIsValid = true;
				}

				i++;
				if (i == reglen) break;

				if (nIsValid == true)
				{
					nCharCount++;
					if ((nCharCount % nMod) == 0)
						d[j++] = (byte)'-';
				}
			}
		}

		return nReturn;
	}

	private boolean RegCodeV1Verify(byte[] szPureCode, long uNameHash,
									RegData out)
	{
		long uBits1, uBits2, uRegNameHash, uSwapChunkSize;
		boolean ret = false;
		byte[] halfPureCode = new byte[REG_CODE_LEN];

		for (int i = 8; i < reglen; i++)
			halfPureCode[i-8] = szPureCode[i];

		uBits1 = B12Decode(szPureCode);
		uBits2 = B12Decode(halfPureCode);

		uBits1 ^= XOR_1;
		uBits2 ^= XOR_2;

		uSwapChunkSize = uBits2 >> 23;
		if(uSwapChunkSize > 6)
			ret = false;
		else
		{
			uBits1 = SwapBits(uBits1, uSwapChunkSize, 23);
			uBits2 = SwapBits(uBits2, uSwapChunkSize, 23);
		}

		uRegNameHash = ((uBits1 & 0x07fff00L) << 1) | (uBits2 & 0x1ffL);
		out.policy = (int)(uBits1 & 0xffL);
		out.month = (int)((uBits2 & 0x1fe00L) >> 9);

		if(((uBits2 & 0x60000L) >> 17) != 0)
			ret = false;

		if(TwoBitCrc(out.policy + (out.month << 8)) != (uBits1 >> 23))
			ret = false;

		if(uRegNameHash != uNameHash)
			ret = false;

		return ret;
	}

	public boolean Verify()
	{
		boolean retv = true;
		int nCodeType;
		long uNameHash, uRegNameHash, uBits1, uBits2, uSwapChunkSize;
		byte[] halfPureCode = new byte[REG_CODE_LEN];

		nCodeType = RegCodeV3Canonicalize(szCode, szPureCode);

		if(nCodeType == REG_CODE_BAD)
			return false;

		if (namlen < 1)
			retv = false;

		uNameHash = XORMD4Hash(szName, namlen, XOR_NAME) & 0xffffff;

		if(nCodeType == REG_CODE_V1)
		{
			RegData tmp = new RegData();
			boolean retv1 = RegCodeV1Verify(szPureCode, uNameHash, tmp);
			month = tmp.month;
			policy = tmp.policy;
			return retv1;
		}

		for (int i = 10; i < reglen; i++)
			halfPureCode[i-10] = szPureCode[i];

		uBits1 = B10Decode(szPureCode);
		uBits2 = B10Decode(halfPureCode);

		uBits1 ^= XOR_1;
		uBits2 ^= XOR_2;
		
		uSwapChunkSize = (uBits2 >> 24) * 2;

		if(uSwapChunkSize > 6)
			retv = false;
		else
		{
			uBits1 = SwapBits(uBits1, uSwapChunkSize, 26);
			uBits2 = SwapBits(uBits2, uSwapChunkSize, 22);
		}
  
		uRegNameHash = ((uBits1 & 0x03fff00) << 2) | (uBits2 & 0x3ff);
		policy = (int)(uBits1 & 0xff);
		month = (int)((uBits2 & 0x3fc00) >> 10);

		if (((uBits2 & 0xc0000) >> 18) != 0)
			retv = false;

		if (FourBitCrc(policy + (month << 8) + ((uBits2 & 0xf00000) >> 4)) != 
			(uBits1 >> 22))
			retv = false;

		if(uRegNameHash != uNameHash)
			retv = false;

		return retv;
    }

	public boolean Encode(String name, int nMonth, int nPolicy, int nRandom)
	{
		long uBits1, uBits2, uNameHash;

		if (name.length() < 1) return false;

		month = nMonth;
		policy = nPolicy;
		szName = name.getBytes();
		namlen = name.length();

		uNameHash = XORMD4Hash(szName, namlen, XOR_NAME);

		uBits1 = (policy | ((uNameHash & 0xfffc00) >> 2)) |
				 (FourBitCrc(policy + (month << 8) + 
				 ((nRandom & 0xf) << 16)) << 22);

		uBits2 = ((uNameHash & 0x3ff) | (month << 10) | 
				  (REG_CODE_SCHEME_VERSION << 18)) |
				  ((nRandom & 0x3f) << 20);

		uBits1 = SwapBits(uBits1, (uBits2 >> 24)*2, 26);
		uBits2 = SwapBits(uBits2, (uBits2 >> 24)*2, 22);

		

		return true;
	}

	private void B10Encode(byte[] dest, long val)
	{
		long uMult, c, n;

		uMult = 10000000;
		for (int i = 0; i < 9; i++)
		{
			n = val / uMult;
			c = (9 - n) + '0';
			dest[i] = (byte)c;
			if (i == 3)
			{
				i++;
				dest[i] = (byte)'-';
			}
			val -= n * uMult;
			uMult /= 10;
		}
	}

    public int GetPolicy()
	{
		return policy;
    }

    public int GetMonth()
	{
		return month;
    }

	public void SetName(String name)
	{
		szName = name.getBytes();
		namlen = name.length();
	}

	public void SetRegCode(String code)
	{
		szCode = code.getBytes();
		reglen = code.length();

		szPureCode = new byte[REG_CODE_LEN];
	}
}
