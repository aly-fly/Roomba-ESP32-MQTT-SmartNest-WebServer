unit Roomba1_main;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Serial_comm, Vcl.StdCtrls, Vcl.ExtCtrls;

type
  TForm1 = class(TForm)
    btnIsciCom: TButton;
    lstComPorti: TListBox;
    btnOpen: TButton;
    btnClose: TButton;
    pnlRoomba: TGroupBox;
    mm1: TMemo;
    btn1: TButton;
    tmr1: TTimer;
    btn2: TButton;
    btn3: TButton;
    btn4: TButton;
    btn5: TButton;
    btn6: TButton;
    btn7: TButton;
    btn8: TButton;
    btn9: TButton;
    cb1: TCheckBox;
    btnClr: TButton;
    procedure btnIsciComClick(Sender: TObject);
    procedure btnOpenClick(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
    procedure tmr1Timer(Sender: TObject);
    procedure btn1Click(Sender: TObject);
    procedure btn2Click(Sender: TObject);
    procedure btn3Click(Sender: TObject);
    procedure btn4Click(Sender: TObject);
    procedure btn5Click(Sender: TObject);
    procedure btn6Click(Sender: TObject);
    procedure btn7Click(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure btn8Click(Sender: TObject);
    procedure btn9Click(Sender: TObject);
    procedure cb1Click(Sender: TObject);
    procedure btnClrClick(Sender: TObject);
  private
    izbranaComNaprava : Integer;
    poslji : AnsiString;
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

procedure TForm1.btn1Click(Sender: TObject);
begin
  mm1.Lines.Add('---- start ---');
  poslji := ansichar(128);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);
end;

procedure TForm1.btn2Click(Sender: TObject);
begin
  mm1.Lines.Add('---- reset ---');
  poslji := ansichar(7);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);

  (*
  bl-start
  STR730
  bootloader id: #x47145443 85E4CFFF
  bootloader info rev: #xF000
  bootloader rev: #x0001
  2010-05-18-1046-L

  Roomba by iRobot!
  str730
  2015-12-07-1444-L
  battery-current-zero 255

  UI Board SVN Rev: 1371

  2015-12-07-1444-L
  r3-orion/tags/release-3.4.2:1914 CLEAN

  bootloader id: 4714 5443 85E4 CFFF
  assembly: 3.8
  revision: 2
  series: 700-series
  flash version: 10
  flash info crc passed: 1


  processor-sleep

  charger-wakeup
  slept for 0 min 1 sec
  start-charge: 2015-12-07-1444-L
  Determining battery type
  estimate-battery-level-from-voltage 2696 mAH 17422 mV

  bat:   min 0  sec 1  mV 17395  mA -47  tenths-deg-C 294  mAH 2696  state 1  mode 0

  bat:   min 0  sec 2  mV 17395  mA -47  tenths-deg-C 293  mAH 2696  state 1  mode 0
  do-charging-full @ min 0

  Performing charger self-test
  bat:   min 0  sec 3  mV 17395  mA -55  tenths-deg-C 292  mAH 2696  state 4  mode 7

  Charging FET test passed

  bat:   min 0  sec 4  mV 17506  mA 891  tenths-deg-C 291  mAH 2696  state 8  mode 7

  *)
end;

procedure TForm1.btn3Click(Sender: TObject);
begin
  mm1.Lines.Add('---- stop ---');
  poslji := ansichar(173);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);
end;

procedure TForm1.btn4Click(Sender: TObject);
begin
  mm1.Lines.Add('---- power off ---');
  poslji := ansichar(133);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);
end;

procedure TForm1.btn5Click(Sender: TObject);
begin
  mm1.Lines.Add('---- clean ---');
  poslji := ansichar(135);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);
end;

procedure TForm1.btn6Click(Sender: TObject);
begin
  mm1.Lines.Add('---- dock ---');
  poslji := ansichar(143);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);
end;

procedure TForm1.btn7Click(Sender: TObject);
var prejeto : AnsiString;
  txt : string;
  i : Integer;
begin
  tmr1.Enabled := False;
  mm1.Lines.Add('---- sensors 3 ---');
  poslji := ansichar(142) + ansichar(3);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);
  if ComPortWaitRead(USBnaprave[izbranaComNaprava], 10, Prejeto) then
    begin
(*
0 Not charging
1 Reconditioning Charging
2 Full Charging
3 Trickle Charging
4 Waiting
5 Charging Fault Condition
*)
    case byte(prejeto[1]) of
      0 : txt := 'Not charging';
      1 : txt := 'Reconditioning Charging';
      2 : txt := 'Full Charging';
      3 : txt := 'Trickle Charging';
      4 : txt := 'Waiting';
      5 : txt := 'Charging Fault Condition';
    else txt := 'UNKNOWN';
    end;
    mm1.Lines.Add('Charging State ' + IntToStr(byte(prejeto[1])) + '  / ' + txt);
    mm1.Lines.Add('Voltage ' + IntToStr(byte(prejeto[2]) SHL 8 + byte(prejeto[3])));
    mm1.Lines.Add('Current ' + IntToStr(shortint(byte(prejeto[4]) SHL 8 + byte(prejeto[5]))));
    mm1.Lines.Add('Temperature ' + IntToStr(byte(prejeto[6])));
    mm1.Lines.Add('Charge ' + IntToStr(byte(prejeto[7]) SHL 8 + byte(prejeto[8])));
    mm1.Lines.Add('Capacity ' + IntToStr(byte(prejeto[9]) SHL 8 + byte(prejeto[10])));
    end;
(*
0 3 6 100 21 Charging State 1 0 - 6
0 3 6 100 22 Voltage 2 0 - 65535 mV
0 3 6 100 23 Current 2 -32768 - 32767 mA
0 3 6 100 24 Temperature 1 -128 - 127 deg C
0 3 6 100 25 Battery Charge 2 0 - 65535 mAh
0 3 6 100 26 Battery Capacity 2 0 - 65535 mAh
*)

  mm1.Lines.Add('---- sensors 2 ---');
  poslji := ansichar(142) + ansichar(2);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);
  if ComPortWaitRead(USBnaprave[izbranaComNaprava], 6, Prejeto) then
    begin
    mm1.Lines.Add('Distance ' + IntToStr(shortint(byte(prejeto[3]) SHL 8 + byte(prejeto[4]))));
    mm1.Lines.Add('Angle ' + IntToStr(shortint(byte(prejeto[5]) SHL 8 + byte(prejeto[6]))));
    end;
(*
0 2 6 100 17 Ir Opcode 1 0 - 255
0 2 6 100 18 Buttons 1 0 - 255
0 2 6 100 19 Distance 2 -32768 - 32767 mm
0 2 6 100 20 Angle 2 -32768 - 32767 degrees
*)

  mm1.Lines.Add('---- sensors 0 ---');
  poslji := ansichar(142) + ansichar(0);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);
  if ComPortWaitRead(USBnaprave[izbranaComNaprava], 26, Prejeto) then
    begin
    for i := 1 to Length(prejeto) do mm1.Lines.Add(IntToHex(byte(prejeto[i]), 2))
    end;
(*
Packet Group Membership Packet Name Bytes Value Range Units
0 1 6 100 7 Bumps Wheeldrops 1 0 - 15
0 1 6 100 8 Wall 1 0 - 1
0 1 6 100 9 Cliff Left 1 0 - 1
0 1 6 100 10 Cliff Front Left 1 0 - 1
0 1 6 100 11 Cliff Front Right 1 0 - 1
0 1 6 100 12 Cliff Right 1 0 - 1
0 1 6 100 13 Virtual Wall 1 0 - 1
0 1 6 100 14 Overcurrents 1 0 - 29
0 1 6 100 15 Dirt Detect 1 0 - 255
0 1 6 100 16 Unused 1 1 0 - 255
0 2 6 100 17 Ir Opcode 1 0 - 255
0 2 6 100 18 Buttons 1 0 - 255
0 2 6 100 19 Distance 2 -32768 - 32767 mm
0 2 6 100 20 Angle 2 -32768 - 32767 degrees
0 3 6 100 21 Charging State 1 0 - 6
0 3 6 100 22 Voltage 2 0 - 65535 mV
0 3 6 100 23 Current 2 -32768 - 32767 mA
0 3 6 100 24 Temperature 1 -128 - 127 deg C
0 3 6 100 25 Battery Charge 2 0 - 65535 mAh
0 3 6 100 26 Battery Capacity 2 0 - 65535 mAh
*)
  tmr1.Enabled := True;
end;

procedure TForm1.btn8Click(Sender: TObject);
begin
  mm1.Lines.Add('---- passive ---');
  poslji := ansichar(130);
  ComPortWrite(USBnaprave[izbranaComNaprava], poslji);
end;

procedure TForm1.btn9Click(Sender: TObject);
begin
  ComPortCTSsignal(usbnaprave[izbranaComNaprava], True);
  sleep(200);
  ComPortCTSsignal(usbnaprave[izbranaComNaprava], False);
  sleep(200);
  ComPortCTSsignal(usbnaprave[izbranaComNaprava], True);
end;

procedure TForm1.btnCloseClick(Sender: TObject);
begin
  ComPortClose(usbnaprave[izbranaComNaprava]);
end;

procedure TForm1.btnClrClick(Sender: TObject);
begin
  mm1.Clear;
end;

procedure TForm1.btnIsciComClick(Sender: TObject);
var i : Integer;
begin
  IsciCOMporteVregistru;
  lstcomporti.clear;
  for i := 0 to Length(USBnaprave)-1 do
    lstComPorti.Items.Add(usbnaprave[i].Vrednost + ' ' + usbnaprave[i].Ime);
  btnOpen.Enabled := lstComPorti.Count > 0;
  if lstComPorti.Count > 0 then lstComPorti.Selected[0] := True;
end;

procedure TForm1.btnOpenClick(Sender: TObject);
  var i : integer;

begin
  pnlRoomba.Visible := False;
  izbranaComNaprava := -1;
  for i := 0 to lstComPorti.Count-1 do
    if lstComPorti.Selected[i] then izbranaComNaprava := i;
  if izbranaComNaprava < 0 then exit;

  ComPortOpen(USBnaprave[izbranaComNaprava]);
  if NOT USBnaprave[izbranaComNaprava].PortNaVoljo then
    begin
    ShowMessage('port ni na voljo');
    exit;
    end;

  USBnaprave[izbranaComNaprava].Baudrate := 115200;
  ComPortSetBaudRate(USBnaprave[izbranaComNaprava]);
  pnlRoomba.Visible := True;
  tmr1.Enabled := True;
end;

procedure TForm1.cb1Click(Sender: TObject);
begin
  ComPortCTSsignal(usbnaprave[izbranaComNaprava], cb1.Checked);
end;

procedure TForm1.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  btnCloseClick(nil);
end;

procedure TForm1.tmr1Timer(Sender: TObject);
var   prejeto : ansistring;
begin
  tmr1.Enabled := False;
  ComPortWaitRead(USBnaprave[izbranaComNaprava], 100, Prejeto);
  if prejeto <> '' then mm1.Lines.Add(prejeto);
  tmr1.Enabled := true;
end;

end.
