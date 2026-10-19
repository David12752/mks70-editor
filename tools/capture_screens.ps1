param(
    [string]$ExePath = "c:\\Users\\User\\Desktop\\synths editors\\MKS70 Editor\\Builds\\VisualStudio2022\\x64\\Debug\\Standalone Plugin\\MKS70 Editor.exe",
    [string]$OutDir = "c:\\Users\\User\\Desktop\\synths editors\\MKS70 Editor\\artifacts\\screens"
)

# Create output directory
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

# Add C# helper for window manipulation and screenshot
Add-Type -Language CSharp -ReferencedAssemblies System.Drawing -TypeDefinition @"
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public static class Win32Screen
{
    [DllImport("user32.dll")] static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);
    [DllImport("user32.dll")] static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

    const int SW_RESTORE = 9;
    static readonly IntPtr HWND_TOP = IntPtr.Zero;
    const uint SWP_SHOWWINDOW = 0x0040;

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }

    public static void ResizeAndCapture(IntPtr h, int width, int height, string file)
    {
        ShowWindow(h, SW_RESTORE);
        SetForegroundWindow(h);
        SetWindowPos(h, HWND_TOP, 100, 100, width, height, SWP_SHOWWINDOW);
        System.Threading.Thread.Sleep(500);
        RECT r; GetWindowRect(h, out r);
        int w = r.Right - r.Left; int hgt = r.Bottom - r.Top;
        using (var bmp = new Bitmap(w, hgt))
        using (var g = Graphics.FromImage(bmp))
        {
            g.CopyFromScreen(r.Left, r.Top, 0, 0, new Size(w, hgt));
            bmp.Save(file, ImageFormat.Png);
        }
    }
}
"@

# Start the app
$proc = Start-Process -FilePath $ExePath -PassThru
Start-Sleep -Seconds 2

# Wait for main window
for ($i=0; $i -lt 40 -and $proc.MainWindowHandle -eq 0; $i++) { Start-Sleep -Milliseconds 200; $proc.Refresh() }
if ($proc.MainWindowHandle -eq 0) { Write-Error "No main window handle found."; exit 1 }

# Capture sizes
$sizes = @(
    @{ Name = "small"; Width = 1024; Height = 840 },
    @{ Name = "medium"; Width = 1280; Height = 900 },
    @{ Name = "large"; Width = 1600; Height = 1000 }
)

foreach ($s in $sizes) {
    $file = Join-Path $OutDir ("mks70_" + $s.Name + ".png")
    [Win32Screen]::ResizeAndCapture($proc.MainWindowHandle, [int]$s.Width, [int]$s.Height, $file)
    Write-Host ("Saved " + $file)
}

# Prepare clipping log template on first run
$clipLog = Join-Path $PSScriptRoot "..\\docs\\CLIPPING_LOG.md"
New-Item -ItemType Directory -Force -Path (Split-Path $clipLog -Parent) | Out-Null
if (-not (Test-Path $clipLog)) {
@"
# Clipping/Elide Log

Record any clipping/overlap observed in screenshots. Use coordinates if possible.

- small (1024x840):
  - [ ]
- medium (1280x900):
  - [ ]
- large (1600x1000):
  - [ ]

Notes:
- Prefer changing layout via Theme::Space and consistent row heights.
- For long labels, add ellipsis + tooltip.
"@ | Set-Content -Path $clipLog -Encoding UTF8
}

# Keep the app running for manual review; comment to close automatically
# Stop-Process -Id $proc.Id
