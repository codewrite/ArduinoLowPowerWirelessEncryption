using System;
using System.IO.Ports;

public class Serial : IDisposable
{
    private SerialPort port;
    private bool _disposed = false;
    
    public byte[] ReceiveBuffer { get; set; }
    public int ReceiveLength { get; set; }

    public Serial()
    {
        ReceiveBuffer = new byte[256];
        port = new SerialPort("/dev/ttyAMA0", 9600, Parity.None, 8, StopBits.One);
        port.Open();
    }

    ~Serial() => Dispose(false);

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (_disposed)
        {
            return;
        }

        if (disposing)
        {
            port.Close();
        }

        _disposed = true;
    }

    public bool Read()
    {
        bool LfReceived = false;
        int c = port.BytesToRead;
        if (c > 0)
        {
            for (int i = 0; i < c; i++)
            {
            if (ReceiveLength + 1 >= 256)
            {
                ReceiveLength = 0; // Buffer overrun - no LF received
            }
            int chr = port.ReadByte();
            if (chr < 0)
            {
                break;  // No char - but this shouldn't really happen
            }
            else
            {
                ReceiveBuffer[ReceiveLength] = (byte)chr;
            }
            if (ReceiveBuffer[ReceiveLength] == 0x0a)
            {
                ReceiveBuffer[ReceiveLength] = 0;
                LfReceived = true;  // LF terminates command
                break;
            }
            else if (ReceiveBuffer[ReceiveLength] == 0x0d)
            {
                // ignore CR
            }
            else
            {
                ReceiveLength++;
            }
            }
        }
        return LfReceived;
    }
}
