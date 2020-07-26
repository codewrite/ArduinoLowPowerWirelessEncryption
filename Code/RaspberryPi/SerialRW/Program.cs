using System;
using System.Text;
using System.Threading;
using CommandLine;

namespace SerialRW
{
    public class Program
    {
        public class Options
        {
            [Option('v', "verbose", Required = false, HelpText = "Set output to verbose messages.")]
            public bool Verbose { get; set; }
            [Option("decrypt", HelpText = "Decrypt")]
            public bool Decrypt { get; set; }
        }

        public static void Main(string[] args)
        {
            bool decrypt = false;
            Parser.Default.ParseArguments<Options>(args)
                .WithParsed<Options>(o =>
                {
                    if (o.Decrypt)
                        decrypt = true;
                });

            var encryption = new Encryption();
            using (var serial = new Serial())
            {
                while (true)
                {
                    while (!serial.Read())
                    {
                        Thread.Sleep(100);
                    }
                    var cipherText = Encoding.UTF8.GetString(serial.ReceiveBuffer, 0, serial.ReceiveLength);
                    serial.ReceiveLength = 0;
                    if (decrypt)
                    {
                        var decryptedMessage = encryption.DecodeString(cipherText);
                        Console.WriteLine(decryptedMessage.plainText);
                        if (!encryption.Poly1305Match(decryptedMessage.plainText, decryptedMessage.shortPoly1305))
                        {
                            Console.WriteLine("Last message MAC invalid");
                        }
                        if (!encryption.IVGreaterThanLast())
                        {
                            Console.WriteLine("Last IV less or equal to previously received IV");
                        }
                    }
                    else
                    {
                        Console.WriteLine(cipherText);
                    }
                }
            }
        }
    }
}
