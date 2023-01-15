program Roomba1;

uses
  Vcl.Forms,
  Roomba1_main in 'Roomba1_main.pas' {Form1},
  Serial_comm in '..\..\..\..\RLSHUB_SWD\RLS_LIB\Serial_comm.pas',
  Orodja in '..\..\..\..\RLSHUB_SWD\RLS_LIB\Orodja.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
