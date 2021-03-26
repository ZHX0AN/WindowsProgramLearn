using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.Runtime.InteropServices;


namespace _05_GetUserInfoCSharp
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            this.textBox1.Clear();

            //微信进程
            Process WxProcess = null;
            //WeChatWin.dll基址
            IntPtr WeChatWinBaseAddress = IntPtr.Zero;
            //微信版本
            String WeChatVersion = "";
            Process[] processes = Process.GetProcesses();

            foreach (Process process in processes)
            {
                if (process.ProcessName == "WeChat")
                {
                    WxProcess = process;
                    this.textBox1.AppendText("微信已找到！" + Environment.NewLine);
                    this.textBox1.AppendText("微信句柄:\t" + "0x" + ((int)(process.Handle)).ToString("X8") + Environment.NewLine);
                    foreach (ProcessModule processModule in process.Modules)
                    {
                        if (processModule.ModuleName == "WeChatWin.dll")
                        {
                            WeChatWinBaseAddress = processModule.BaseAddress;
                            this.textBox1.AppendText("微信基址:\t" + "0x" + ((int)(processModule.BaseAddress)).ToString("X8") + Environment.NewLine);

                            WeChatVersion = processModule.FileVersionInfo.FileVersion;
                            this.textBox1.AppendText("微信版本:\t" + processModule.FileVersionInfo.FileVersion + Environment.NewLine);

                            break;
                        }
                    }
                    break;
                }
            }

            if (WxProcess == null)
            {
                this.textBox1.AppendText("微信没有找到！");
                return;
            }



            //微信昵称
            int WxNickNameAddress = 0x7AD00000 + 0x18A35FC;
            this.textBox1.AppendText("微信昵称地址:\t" + "0x" + ((int)(WxNickNameAddress)).ToString("X8") + Environment.NewLine);
            this.textBox1.AppendText("微信昵称:\t" + GetString(WxProcess.Handle, (IntPtr)WxNickNameAddress) + Environment.NewLine);

            //微信ID wxid
            int WxIdAddress = 0x7AD00000 + 0x18A3584;
            this.textBox1.AppendText("微信ID地址:\t" + "0x" + ((int)(WxIdAddress)).ToString("X8") + Environment.NewLine);
            this.textBox1.AppendText("微信ID:\t" + GetString(WxProcess.Handle, (IntPtr)(GetAddress(WxProcess.Handle, (IntPtr)WxIdAddress))) + Environment.NewLine);

        }


        String GetString(IntPtr hProcess, IntPtr lpBaseAddress, int nSize = 100)
        {
            byte[] data = new byte[nSize];
            if (ReadProcessMemory(hProcess, lpBaseAddress, data, nSize, 0) == 0)
            {
                //读取内存失败！
                return "";
            }
            String result = "";
            String TempString = Encoding.UTF8.GetString(data);
            foreach (char item in TempString)
            {
                if (item == '\0')
                {
                    break;
                }
                result += item.ToString();
            }
            return result;

        }

        int GetAddress(IntPtr hProcess, IntPtr lpBaseAddress)
        {
            byte[] data = new byte[4];
            if (ReadProcessMemory(hProcess, lpBaseAddress, data, 4, 0) == 0)
            {
                //读取内存失败！
                return 0;
            }

            String Hex = data[3].ToString("x2") +
                data[2].ToString("x2") +
                data[1].ToString("x2") +
                data[0].ToString("x2");
            return int.Parse(Hex, System.Globalization.NumberStyles.HexNumber);
        }


        [DllImport("Kernel32.dll")]
        //BOOL ReadProcessMemory(
        //  HANDLE hProcess,
        //  LPCVOID lpBaseAddress,
        //  LPVOID lpBuffer,
        //  SIZE_T nSize,
        //  SIZE_T* lpNumberOfBytesRead
        //);
        public static extern int ReadProcessMemory(
              IntPtr hProcess,  //正在读取内存的进程句柄。句柄必须具有PROCESS_VM_READ访问权限。
              IntPtr lpBaseAddress,    //指向要从中读取的指定进程中的基址的指针。在发生任何数据传输之前，系统会验证基本地址和指定大小的内存中的所有数据是否都可以进行读访问，如果无法访问，则该函数将失败。
              byte[] lpBuffer,  //指向缓冲区的指针，该缓冲区从指定进程的地址空间接收内容。
              int nSize,    //要从指定进程读取的字节数。
              int lpNumberOfBytesRead //指向变量的指针，该变量接收传输到指定缓冲区的字节数。如果lpNumberOfBytesRead为NULL，则忽略该参数。
            );

        private void BTN_UTF8_Click(object sender, EventArgs e)
        {
            byte[] b1 = { 0xa9, 0x52, 0x4b, 0x62, 0x7A };
            String TempString = Encoding.UTF8.GetString(b1);
            this.textBox1.AppendText(TempString + Environment.NewLine);

            byte[] b2 = { 0xE5, 0x8A, 0xA9, 0xE6, 0x89, 0x8B, 0x7A };
            String TempString2 = Encoding.UTF8.GetString(b1);
            this.textBox1.AppendText(TempString2 + Environment.NewLine);

        }

        private void Emoji_TEST_Click(object sender, EventArgs e)
        {


            string paramContent = @" 中间有中文 [e]1f604[/e]也可以[e]1f612[/e][e]1f60c[/e][e]1f60c[/e][e]1f60d[/e][e]1f616[/e][e]1f632[/e][e]1f632[/e][e]1f632[/e][e]1f632[/e][e]1f616[/e][e]1f632[/e]";

            string s = GetEmoji(paramContent);
            this.textBox1.AppendText(s + Environment.NewLine);

        }

        public string GetEmoji(string paramContent)
        {
            string paramContentR = paramContent.Replace("[e]", "\\u").Replace("[/e]", "");


            var unicodehex = new char[6] { '0', '0', '0', '0', '0', '0' };

            StringBuilder newString = new StringBuilder(2000);
            StringBuilder tempEmojiSB = new StringBuilder(20);
            StringBuilder tmps = new StringBuilder(5);

            int ln = paramContent.Length;
            for (int index = 0; index < ln; index++)
            {
                int i = index; //把指针给一个临时变量,方便出错时,现场恢复.
                try
                {

                    if (paramContent[i] == '[')
                    {
                        //预测
                        if (paramContent[i + 1] == 'e')
                        {
                            if (paramContent[i + 2] == ']') //[e]的后面4位是 unicode 的16进制数值.
                            {
                                i = i + 3; //前进3位 

                                i = ChangUnicodeToUTF16(paramContent, tempEmojiSB, tmps, i);

                                if (paramContent[i] == '-')//向前探测1位 看看是否双字符 形如1f1e7-1f1ea 
                                {
                                    i++;
                                    i = ChangUnicodeToUTF16(paramContent, tempEmojiSB, tmps, i);

                                };

                                if (paramContent[i] == '[')
                                {
                                    if (paramContent[i + 1] == '/')
                                    {
                                        if (paramContent[i + 2] == 'e')
                                        {
                                            if (paramContent[i + 3] == ']')
                                            {
                                                i = i + 3; //再前进4位

                                                index = i;                             //识别转换成功
                                                newString.Append(tempEmojiSB.ToString());   //识别转换成功
                                                tempEmojiSB.Clear();
                                                continue;
                                            }
                                        }
                                    }
                                }

                            }
                        }
                    }

                    index = i;

                }
                catch (Exception ex)
                {
                    //解析失败仍然继续吃.
                }
                newString.Append(paramContent[index]);

            }
            return newString.ToString();
        }


        public int ChangUnicodeToUTF16(string paramContent, StringBuilder tempSB, StringBuilder tmps, int i)
        {
            for (int maxln = 0; maxln < 20; maxln++)
            {
                if (paramContent[i] != '-' && paramContent[i] != '[')
                {  //向前探测1位
                    tmps.Append(paramContent[i]);
                    i++;
                }
                else
                {
                    break;
                }
            }


            tempSB.Append(EmojiCodeToUTF16String(tmps.ToString()));

            tmps.Clear();
            return i;
        }


        /// <returns>EMOJI字符对应的UTF16编码16进制整形表示</returns>
        public Int32 EmojiToUTF16(Int32 V, bool LowHeight = true)
        {

            Int32 Vx = V - 0x10000;

            Int32 Vh = Vx >> 10;//取高10位部分
            Int32 Vl = Vx & 0x3ff; //取低10位部分
            //Response.Write("Vh:"); Response.Write(Convert.ToString(Vh, 2)); Response.Write("<br/>"); //2进制显示
            //Response.Write("Vl:"); Response.Write(Convert.ToString(Vl, 2)); Response.Write("<br/>"); //2进制显示

            Int32 wh = 0xD800; //結果的前16位元初始值,这个地方应该是根据Unicode的编码规则总结出来的数值.
            Int32 wl = 0xDC00; //結果的後16位元初始值,这个地方应该是根据Unicode的编码规则总结出来的数值.
            wh = wh | Vh;
            wl = wl | Vl;
            //Response.Write("wh:"); Response.Write(Convert.ToString(wh, 2)); Response.Write("<br/>");//2进制显示
            //Response.Write("wl:"); Response.Write(Convert.ToString(wl, 2)); Response.Write("<br/>");//2进制显示
            if (LowHeight)
            {
                return wl << 16 | wh;   //低位左移16位以后再把高位合并起来 得到的结果是UTF16的编码值   //适合低位在前的操作系统 
            }
            else
            {
                return wh << 16 | wl; //高位左移16位以后再把低位合并起来 得到的结果是UTF16的编码值   //适合高位在前的操作系统
            }


        }


        /// <summary>
        /// 字符串形式的 Emoji 16进制Unicode编码  转换成 UTF16字符串 能够直接输出到客户端
        /// </summary>
        /// <param name="EmojiCode"></param>
        /// <returns></returns>
        public string EmojiCodeToUTF16String(string EmojiCode)
        {
            if (EmojiCode.Length != 4 && EmojiCode.Length != 5)
            {
                throw new ArgumentException("错误的 EmojiCode 16进制数据长度.一般为4位或5位");
            }
            //1f604
            int EmojiUnicodeHex = int.Parse(EmojiCode, System.Globalization.NumberStyles.HexNumber);

            //1f604对应 utf16 编码的int
            Int32 EmojiUTF16Hex = EmojiToUTF16(EmojiUnicodeHex, true);             //这里字符的低位在前.高位在后.
            //Response.Write(Convert.ToString(lon, 16)); Response.Write("<br/>"); //这里字符的低位在前.高位在后. 
            var emojiBytes = BitConverter.GetBytes(EmojiUTF16Hex);                     //把整型值变成Byte[]形式. Int64的话 丢掉高位的空白0000000   

            return Encoding.Unicode.GetString(emojiBytes);
        }

    }
}
