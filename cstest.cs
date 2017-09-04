class Program
{
    static int dlopenAddr = 0xEAD0; //opendl地址
    static int InitArrayAddr = 0x57480-0x1000; //此处IDA的地址总是比UE中的多0x1000
    static int asmAddr = 0x51F00; //代码插入位置
     
    static byte[] asm = { 0xFF,0x40,0x2D,0xE9,0x18,0x00,0x9F,0xE5,0x00,0x10,0xA0,0xE3,0x00,0x00,0x8F,0xE0,
                            0x01,0xF7,0xFF,0xEB,0xFF,0x40,0xBD,0xE8,0x08,0x00,0x9F,0xE5,0x00,0x00,0x8F,0xE0,
                            0x10,0xFF,0x2F,0xE1,0x9C,0x68,0x03,0x00,0x09,0xBF,0x01,0x00,0x00,0x00,0x00,0x00};
    static int StrAddr = asmAddr + 0x40;
    static void Main(string[] args)
    {
        BinaryReader br = null;
        try
        {
            br = new BinaryReader(new FileStream("libc.so", FileMode.Open));
        }
        catch (IOException e)
        {
            Console.WriteLine(e.Message + "Cannot open file.");
            return;
        }
        int length = Convert.ToInt32(br.BaseStream.Length);
        byte[] data = br.ReadBytes(length);
        br.Close();
        byte[] LibCkisSo = System.Text.Encoding.ASCII.GetBytes("libckis.so");
        Array.ConstrainedCopy(LibCkisSo, 0, data, StrAddr, LibCkisSo.Length);
        data[StrAddr + LibCkisSo.Length] = 0;
        int JmpReturnAddr = System.BitConverter.ToInt32(data, InitArrayAddr);
        byte[] byAsmAddr = System.BitConverter.GetBytes(asmAddr);
        Array.ConstrainedCopy(byAsmAddr, 0, data, InitArrayAddr, byAsmAddr.Length);
        byte[] JmpOffset = System.BitConverter.GetBytes(JmpReturnAddr - asmAddr - 0x24);
        Array.ConstrainedCopy(JmpOffset, 0, asm, 0x28, JmpOffset.Length);
        byte[] StrOffset = System.BitConverter.GetBytes(StrAddr - asmAddr - 0x14);
        Array.ConstrainedCopy(StrOffset, 0, asm, 0x24, StrOffset.Length);
        byte[] DlopenOffset = System.BitConverter.GetBytes((dlopenAddr - asmAddr - 0x18) / 4);
        Array.ConstrainedCopy(DlopenOffset, 0, asm, 0x10, StrOffset.Length - 1);
        Array.ConstrainedCopy(asm, 0, data, asmAddr, asm.Length);
        BinaryWriter bw = new BinaryWriter(new FileStream("libc_ok.so", FileMode.Create));
        bw.Write(data);
        bw.Close();
    }
}