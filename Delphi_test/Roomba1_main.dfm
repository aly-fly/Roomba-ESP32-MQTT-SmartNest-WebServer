object Form1: TForm1
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Roomba 700'
  ClientHeight = 463
  ClientWidth = 831
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object btnIsciCom: TButton
    Left = 8
    Top = 16
    Width = 75
    Height = 25
    Caption = 'Isci Com'
    TabOrder = 0
    OnClick = btnIsciComClick
  end
  object lstComPorti: TListBox
    Left = 8
    Top = 47
    Width = 244
    Height = 138
    ItemHeight = 13
    TabOrder = 1
  end
  object btnOpen: TButton
    Left = 96
    Top = 16
    Width = 75
    Height = 25
    Caption = 'Open'
    Enabled = False
    TabOrder = 2
    OnClick = btnOpenClick
  end
  object btnClose: TButton
    Left = 177
    Top = 16
    Width = 75
    Height = 25
    Caption = 'Close'
    TabOrder = 3
    OnClick = btnCloseClick
  end
  object pnlRoomba: TGroupBox
    Left = 258
    Top = 16
    Width = 183
    Height = 429
    Caption = '  Roomba  '
    TabOrder = 4
    Visible = False
    object btn1: TButton
      Left = 16
      Top = 16
      Width = 75
      Height = 25
      Caption = 'start'
      TabOrder = 0
      OnClick = btn1Click
    end
    object btn2: TButton
      Left = 16
      Top = 47
      Width = 75
      Height = 25
      Caption = 'reset'
      TabOrder = 1
      OnClick = btn2Click
    end
    object btn3: TButton
      Left = 16
      Top = 392
      Width = 75
      Height = 25
      Caption = 'stop'
      TabOrder = 2
      OnClick = btn3Click
    end
    object btn4: TButton
      Left = 16
      Top = 361
      Width = 75
      Height = 25
      Caption = 'power off'
      TabOrder = 3
      OnClick = btn4Click
    end
    object btn5: TButton
      Left = 16
      Top = 185
      Width = 75
      Height = 25
      Caption = 'clean'
      TabOrder = 4
      OnClick = btn5Click
    end
    object btn6: TButton
      Left = 16
      Top = 216
      Width = 75
      Height = 25
      Caption = 'dock'
      TabOrder = 5
      OnClick = btn6Click
    end
    object btn7: TButton
      Left = 16
      Top = 272
      Width = 75
      Height = 25
      Caption = 'sensors?'
      TabOrder = 6
      OnClick = btn7Click
    end
    object btn8: TButton
      Left = 16
      Top = 154
      Width = 75
      Height = 25
      Caption = 'passive / stop'
      TabOrder = 7
      OnClick = btn8Click
    end
    object btn9: TButton
      Left = 104
      Top = 104
      Width = 75
      Height = 25
      Caption = 'wake up'
      TabOrder = 8
      OnClick = btn9Click
    end
    object cb1: TCheckBox
      Left = 104
      Top = 135
      Width = 97
      Height = 17
      Caption = 'cb1'
      TabOrder = 9
      OnClick = cb1Click
    end
  end
  object mm1: TMemo
    Left = 456
    Top = 18
    Width = 367
    Height = 427
    ScrollBars = ssVertical
    TabOrder = 5
  end
  object btnClr: TButton
    Left = 177
    Top = 201
    Width = 75
    Height = 25
    Caption = 'Clr'
    TabOrder = 6
    OnClick = btnClrClick
  end
  object tmr1: TTimer
    Enabled = False
    Interval = 400
    OnTimer = tmr1Timer
    Left = 16
    Top = 200
  end
end
