                COMMENT *
===========================================================================
FILE    : SOFTLOCK.ASM
SUBJECT : TSR to enable password controlled keyboard locking.

USAGE:

        F:\>SOFTLOCK p=password [t=time] [h=hotkey]

The password must be only letters -- no special characters or control
characters will be accepted. Passwords are case insensitive. The password
may be up to 79 characters long. A password must be provided.

An optional time can be entered at the command line. This is the number
of minutes of keyboard inactivity that softlock will wait before locking
the keyboard. The default value is 1. The maximum value is 60. Non digit
characters in the time are ignored.

An optional hotkey can be entered at the command line. This is the control
character that will trigger an immediate lock. The default is no hotkey.

Typical usage:

        F:\>SOFTLOCK p=verylongpassword t=5 h=y

This will cause the keyboard to lock after 5 minutes of inactivity
(although a ^Y will lock the keyboard at once). The string
"verylongpassword" must be typed at the keyboard to unlock it.

If you invoke SOFTLOCK without a password, it will display a usage message
and refuse to load.

TECHNICAL INFO:

The program hooks the keyboard interrupt. When the keyboard is not locked,
it simply passes control to the real keyboard interrupt handler for normal
processing. If the keyboard is locked, this program takes the
responsibility of reading the keyboard hardware. It converts the scan code
thus obtained to an ASCII code and compares that with the next character in
the password. In any case, when the keyboard is locked, the program
prevents the real keyboard interrupt handler from being notified of
keystrokes.

The program also hooks the timer interrupt so that it can keep track of the
time since the last keyboard interrupt. It does this by incrementing a
counter for each timer tick.

Note that this program discards the transient code segment and the stack
segment when it goes resident. This reduces the size of its resident image.

BUGS:

The program does not lock the mouse. Also, it won't work if loaded before
another program that takes control of the keyboard interrupt (eg Windows).
Finally, if you load SOFTLOCK multiple times, you may have to use several
passwords to unlock the keyboard. I consider this a bug, but some may
consider it a feature.

Please send comments and bug reports to

        Peter Chapin
        P.O. Box 375
        Randolph Center, VT 05061
        chapinp@vscnet.bitnet
===========================================================================
*


;===========================================================================

NO              EQU     0       ; Nice names for boolean constants.
YES             EQU     1

CR              EQU     0Dh     ; Nice names for certain ASCII characters.
LF              EQU     0Ah
BEEP            EQU     07h

;===========================================================================

DataSegment     SEGMENT 'DATA'

                ; Holds the password.
Password        DB      80 DUP (0)
Pass_Pntr       DW      OFFSET Password

                ; Holds the ASCIIZ representation of the lockout time.
Time_String     DB      10 DUP (0)
Time            DW      1080            ; The default time in 1/18 seconds.
                                        ;   This is 1 minute.

                ; Holds the ASCIIZ representation of the hot key.
Hot_String      DB      10 DUP (0)

                ; The location of the real keyboard ISR.
OldKey_ISR      LABEL   DWORD
OldKey_Off      DW      0
OldKey_Seg      DW      0

                ; The location of the real timer ISR.
OldTim_ISR      LABEL   DWORD
OldTim_Off      DW      0
OldTim_Seg      DW      0

Counter         DW      0       ; Keeps track of how many times the timer has been called.
Locked          DW      YES     ; =YES when the keyboard is locked.
Screen_Pntr     DW      0       ; Used for debugging purposes.

                ; This table converts scan codes into ASCII codes for upper
                ;   case letters only. The program must avoid indexing off
                ;   this table.
Scan_Table      DB      16 DUP (0)
                DB      'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'
                DB       4 DUP (0)
                DB      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'
                DB       5 DUP (0)
                DB      'Z', 'X', 'C', 'V', 'B', 'N', 'M'
                ; There are 51 entries in the above table.

Header          DB      CR, LF
                DB      "SOFTLOCK  (Version 1.1b)  April 30, 1992", CR, LF
                DB      "Public Domain Software by Peter Chapin", CR, LF, "$"

UsageMsg        DB      CR, LF, "USAGE: SOFTLOCK p=password [t=time] [h=hotkey]", CR, LF
                DB      "       Items in brackets are optional.", CR, LF, CR, LF
                DB      "PASSWORD => String consisting of only letters.", CR, LF
                DB      "TIME     => Integer number of minutes (1..60).", CR, LF
                DB      "HOTKEY   => Ctrl+letter that will activate lock at once.", CR, LF, CR, LF
                DB      "Softlock NOT installed.", CR, LF, "$"

No_PasswordMsg  DB      BEEP, CR, LF, "No password seen on the command line => Use p=password", CR, LF
                DB      "Softlock NOT installed.", CR, LF, "$"

Bad_OptionMsg   DB      BEEP, CR, LF, "Unrecognized option or illegal command line syntax => Type SOFTLOCK for usage.", CR, LF
                DB      "Softlock NOT installed.", CR, LF, "$"

Bad_PasswordMsg DB      BEEP, CR, LF, "Illegal characters in password => Use only letters.", CR, LF
                DB      "Softlock NOT installed.", CR, LF, "$"

OKMsg           DB      CR, LF, "Installed. Keyboard is now LOCKED", CR, LF, "$"

DataSegment     ENDS

;===========================================================================

RCodeSegment    SEGMENT 'CODE'

                ASSUME  CS:RCodeSegment, DS:DataSegment, SS:StackSegment

                COMMENT *
---------------------------------------------------------------------------
The following procedure is a replacement for the keyboard ISR. If the
keyboard is currently locked, it deals with unlocking issues. If the
keyboard is unlocked, it looks for the hot key to see if it should be
locked immediately. Otherwise, it passed control to the normal keyboard ISR
for regular processing.
---------------------------------------------------------------------------
*

Keyboard_ISR    PROC    FAR
                ; Save the interrupted program's registers.
                push    ax
                push    bx
                push    ds
                push    es

                ; Get at our data segment.
                mov     ax, DataSegment
                mov     ds, ax

                COMMENT !
                ; Debugging code prints a 'k' on the screen for every interrupt.
                mov     ax, 0B800h
                mov     es, ax
                mov     bx, Screen_Pntr
                mov     BYTE PTR es:[bx], 'k'
                add     bx, 2
                cmp     bx, 4000
                jb      K_Debug
                mov     bx, 0
K_Debug:
                mov     Screen_Pntr, bx
!
                ; Is the keyboard locked?
                cmp     Locked, YES
                je      Yep

                ; It wasn't. Reset the counter.
                mov     Counter, 0

                ; Has a hot key been defined?
                cmp     Hot_String, 0
                je      No_HotKey

                ; Has the hot key been pressed?
                mov     ah, Hot_String
                in      al, 60h         ; Read the keyboard hardware.
                cmp     al, 50          ; Scan code too large to be hot key?
                ja      No_HotKey
                mov     bx, OFFSET Scan_Table
                xlat                    ; Translate scan code to ASCII.
                cmp     al, 0           ; Is there a table entry for this letter?
                je      No_HotKey

                ; Was this keystroke the hot key?
                cmp     al, ah
                jne     No_HotKey

                ; See if the Ctrl key is being pressed also.
                mov     ax, 0000h
                mov     es, ax
                mov     al, es:[0417h]  ; Check the shift status byte.
                test    al, 00000100b   ; Bit 2 is the Ctrl status.
                jz      No_HotKey

                ; Lock the keyboard.
                mov     Locked, YES

                ; Acknowledge the keystroke...
                sti                     ; Turn interrupts back on.
                in      al, 61h         ; Ack by pulsing bit 7 of port 61h.
                or      al, 10000000b
                out     61h, al
                and     al, 01111111b
                out     61h, al

                ; ... and get out of this ISR.
                jmp     Wrong_Key


No_HotKey:
                ; Call the real keyboard ISR.
                pushf
                call    OldKey_ISR
                jmp     K_Done

Yep:
                ; It's locked. Is Pass_Pntr pointing at a 0?
                mov     bx, Pass_Pntr
                cmp     BYTE PTR [bx], 0
                jne     Advance_Password

                ; It is! Time to unlock the keyboard...
                mov     Locked, NO
                mov     Counter, 0
                mov     bx, OFFSET Password
                mov     Pass_Pntr, bx
                mov     ax, 0000h       ; Set es to the BIOS memory area.
                mov     es, ax
                and     BYTE PTR es:[0417h], 0F0h
                                        ; Make sure Ctrl, etc are off.

                ; ... and call the real keyboard ISR.
                pushf
                call    OldKey_ISR
                jmp     K_Done

Advance_Password:
                ; The keyboard's locked and there still password to process.
                ;   Now, we've got to deal with reading keys.
                sti                     ; Turn interrupts back on.
                in      al, 60h         ; Check keyboard hardware port.
                mov     ah, al
                in      al, 61h         ; Ack by pulsing bit 7 of port 61h.
                or      al, 10000000b
                out     61h, al
                and     al, 01111111b
                out     61h, al

                COMMENT !
                ; Debugging code prints the scan code on the screen.
                push    ax
                mov     ax, 0B800h
                mov     es, ax
                pop     ax
                mov     bx, Screen_Pntr
                mov     es:[bx], ah
                add     bx, 2
                cmp     bx, 4000
                jb      S_Debug
                mov     bx, 0
S_Debug:
                mov     Screen_Pntr, bx
!
                ; See if the scan code is interesting.
                mov     al, ah
                cmp     al, 50                  ; Is it too big?
                ja      Int_Done
                mov     bx, OFFSET Scan_Table   ; Is it's converted code a zero?
                xlat
                cmp     al, 00h
                je      Int_Done

                ; See if this is the correct next keystroke.
                mov     bx, Pass_Pntr
                cmp     BYTE PTR [bx], al
                jne     Wrong_Key

                ; So far, so good.
                inc     bx
                mov     Pass_Pntr, bx
                jmp     Int_Done

Wrong_Key:
                ; Reset the password pointer. User has to start again.
                mov     bx, OFFSET Password
                mov     Pass_Pntr, bx

Int_Done:
                ; Tell the PIC that this interrupt is over.
                mov     al, 20h
                out     20h, al

K_Done:
                ; Return to the interrupted program.
                pop     es
                pop     ds
                pop     bx
                pop     ax
                iret
Keyboard_ISR    ENDP

                COMMENT *
---------------------------------------------------------------------------
The following procedure is a replacment for the timer ISR. It sets the
keyboard locked flag if the number of timer ticks reaches the trigger
point. In any case, it calls the normal timer ISR for regular processing.
---------------------------------------------------------------------------
*

Timer_ISR       PROC    FAR
                ; Save interrupted program's registers.
                push    ax
                push    bx
                push    ds
                push    es

                ; Get at our data segment.
                mov     ax, DataSegment
                mov     ds, ax

                COMMENT !
                ; Debugging code prints a 't' on the screen for every interrupt.
                mov     ax, 0B800h
                mov     es, ax
                mov     bx, Screen_Pntr
                mov     BYTE PTR es:[bx], 't'
                add     bx, 2
                cmp     bx, 4000
                jb      T_Debug
                mov     bx, 0
T_Debug:
                mov     Screen_Pntr, bx
!

                ; Adjust count records.
                inc     Counter
                mov     ax, Counter
                cmp     ax, Time
                jb      Nope

                ; It's been too long without a counter reset. Lock the keyboard.
                mov     Locked, YES

Nope:
                ; Call the real timer ISR.
                pushf
                call    OldTim_ISR

                ; Return to the interrupted program.
                pop     es
                pop     ds
                pop     bx
                pop     ax
                iret
Timer_ISR       ENDP

RCodeSegment    ENDS

;===========================================================================

TCodeSegment    SEGMENT 'CODE'

                ASSUME  CS:TCodeSegment, DS:DataSegment, SS:StackSegment

                COMMENT *
---------------------------------------------------------------------------
The following procedure takes control from MS-DOS. It scans the command
line, installs the new interrupt service routines and then goes TSR. If
there is any kind of error detected in the command line, the program
terminates without staying resident.
---------------------------------------------------------------------------
*

Start           PROC    NEAR
                ; Get at my data segment
                mov     ax, DataSegment
                mov     ds, ax

                ; Tell the user we're up and running.
                mov     ah, 09
                mov     dx, OFFSET Header
                int     21h

                ; Get set up.
                call    Command_Upper   ; Raise letters in command line to upper case.
                call    Parse_Command   ; Find the password and copy it into DataSegment.
                call    Check_Password  ; Verify that password is only letters.
                call    Hook_Keyboard   ; Install the keyboard ISR.
                call    Hook_Timer      ; Install the timer ISR.

                ; Tell the user that it worked.
                mov     ah, 09
                mov     dx, OFFSET OKMsg
                int     21h

; DEBUG code to terminate without doing a TSR.
;                mov     ah, 4Ch
;                mov     al, 00h
;                int     21h

                ; TSR
                mov     ah, 31h
                mov     al, 00h
                mov     dx, TCodeSegment
                sub     dx, DataSegment
                add     dx, 16
                int     21h
Start           ENDP

                COMMENT *
---------------------------------------------------------------------------
The following procedure skips white space in the string pointed at by
es:[si].
---------------------------------------------------------------------------
*

Skip_White      PROC    NEAR
                ; Loop until es:[si] points at a non white space character.
                cmp     BYTE PTR es:[si], ' '
                je      Advance_Pntr
                cmp     BYTE PTR es:[si], 09h
                jne     Non_White

Advance_Pntr:
                inc     si
                jmp     Skip_White

Non_White:
                ret
Skip_White      ENDP

                COMMENT *
---------------------------------------------------------------------------
The following procedure copies a white space delimited word from es:[si] to
ds:[di]. The end of the string is taken to be a CR character.
---------------------------------------------------------------------------
*

Copy_Word       PROC    NEAR
                ; Loop until CR or white space found, copying es:[si] into [di]
                cmp     BYTE PTR es:[si], CR
                je      Done_Copy
                cmp     BYTE PTR es:[si], ' '
                je      Done_Copy
                cmp     BYTE PTR es:[si], 09h
                je      Done_Copy

                ; It wasn't the end of the word. So copy this byte.
                mov     al, es:[si]
                mov     [di], al

                ; Goto to next byte.
                inc     si
                inc     di
                jmp     Copy_Word

Done_Copy:
                ; Null terminate the word at [di]
                mov     BYTE PTR [di], 0
                ret
Copy_Word       ENDP

                COMMENT *
---------------------------------------------------------------------------
The following procedure takes a string of digits in Time_String and
calculates the number of 1/18th seconds corresponding to the specified
string (assumed to be in units of minutes). The result is left in Time.
Time_String cannot contain a number greater than 60. Otherwise the result
is undefined.

This procedure stops when it finds a non digit. It also defaults to 1
minute's worth of time. This procedure changes no registers.
---------------------------------------------------------------------------
*

Compute_Minutes PROC
                ; Save registers.
                push    ax
                push    bx
                push    cx
                push    dx

                mov     bx, OFFSET Time_String
                mov     al, 0
Multiply_Loop:
                ; At the end of the string?
                cmp     BYTE PTR [bx], 0
                je      Multiply_Done

                ; Is this character a number?
                cmp     BYTE PTR [bx], '0'
                jb      Multiply_Done
                cmp     BYTE PTR [bx], '9'
                ja      Multiply_Done

                ; Get the digit and convert it to a number.
                mov     cl, [bx]
                sub     cl, '0'

                ; Blend it in.
                mov     ch, 10
                imul    ch
                add     al, cl

                ; Next digit.
                inc     bx
                jmp     Multiply_Loop

Multiply_Done:
                ; Make sure the time is not zero.
                cmp     al, 0
                jne     Valid_Value
                mov     ax, 1080
Valid_Value:
                mov     cx, 1080
                imul    cx
                mov     Time, ax

                ; Restore registers.
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                ret
Compute_Minutes ENDP

                COMMENT *
---------------------------------------------------------------------------
The following procedure breaks the command line down into it's parts. It
expects that a password is present as the first thing on the command line.
It checks for correct syntax as defined in the usage message. It will
terminate the program without going TSR if it encounters an error.
---------------------------------------------------------------------------
*

Parse_Command   PROC    NEAR
                ; Initialize pointers.
                mov     si, 0081h

                ; Skip leading white space on the command line.
                call    Skip_White

                ; If there is nothing left. We have an error.
                cmp     BYTE PTR es:[si], CR
                jne     Args_Exist
                mov     ah, 09h
                mov     dx, OFFSET UsageMsg
                int     21h
                mov     ah, 4Ch         ; Terminate program with ERRORLEVEL 1
                mov     al, 01h
                int     21h

Args_Exist:
                ; Make sure first argument is the password.
                cmp     BYTE PTR es:[si], 'P'
                jne     Password_Error
                cmp     BYTE PTR es:[si+1], '='
                je      Password_Exists

Password_Error:
                mov     ah, 09h
                mov     dx, OFFSET No_PasswordMsg
                int     21h
                mov     ah, 4Ch         ; Terminate program with ERRORLEVEL 1
                mov     al, 01h
                int     21h

Password_Exists:
                ; Save password as ASCIIZ string.
                add     si, 2
                mov     di, OFFSET Password
                call    Copy_Word

                ; Loop until there's nothing left on the command line.
CLine_Loop:
                call    Skip_White
                cmp     BYTE PTR es:[si], CR
                je      CLine_Done

                ; Make sure the '=' is present.
                cmp     BYTE PTR es:[si+1], '='
                je      Option_Ok
                mov     ah, 09h
                mov     dx, OFFSET Bad_OptionMsg
                int     21h
                mov     ah, 4Ch         ; Terminate program with ERRORLEVEL 1
                mov     al, 01h
                int     21h

Option_Ok:
                ; See what command line option this is.
                mov     al, es:[si]
                cmp     al, 'T'
                jne     Not_Time
                add     si, 2
                mov     di, OFFSET Time_String
                call    Copy_Word
                call    Compute_Minutes
                jmp     CLine_Loop

Not_Time:
                cmp     al, 'H'
                jne     Not_Hot
                add     si, 2
                mov     di, OFFSET Hot_String
                call    Copy_Word
                jmp     CLine_Loop

Not_Hot:
                mov     ah, 09h
                mov     dx, OFFSET Bad_OptionMsg
                int     21h
                mov     ah, 4Ch         ; Terminate program with ERRORLEVEL 1
                mov     al, 01h
                int     21h

CLine_Done:
                ret
Parse_Command   ENDP

                COMMENT *
---------------------------------------------------------------------------
The following procedure raises the command line to upper case. It assumes
that es points at the PSP.
---------------------------------------------------------------------------
*

Command_Upper   PROC    NEAR
                ; Set si to point at the CR terminated command line.
                mov     si, 0081h

Upper_Loop:
                ; Are we at the end of the command line?
                mov     al, es:[si]
                cmp     al, CR
                je      Upper_Done

                ; Are we looking at a lower case letter?
                cmp     al, 'a'
                jb      Not_Lower
                cmp     al, 'z'
                ja      Not_Lower

                ; Yes. Change it to upper case.
                sub     al, 20h

Not_Lower:
                ; Advance to the next character on the command line.
                mov     es:[si], al
                inc     si
                jmp     Upper_Loop

Upper_Done:
                ret
Command_Upper   ENDP

                COMMENT *
---------------------------------------------------------------------------
The following procedure verifies that the password contains only letters.
It terminates the program without going TSR and prints an appropriate
message if there is an error.
---------------------------------------------------------------------------
*

Check_Password  PROC    NEAR
                ; Point at the null terminated password.
                mov     bx, OFFSET Password

Check_Loop:
                ; Are we at the end of the password string?
                mov     al, [bx]
                cmp     al, 0
                je      Check_Done

                ; Jump to Check_Error if the character is not an upper case letter.
                cmp     al, 'A'
                jb      Check_Error
                cmp     al, 'Z'
                ja      Check_Error

                ; Character ok. Check next character.
                inc     bx
                jmp     Check_Loop

Check_Error:
                ; Character not ok. Print an error message and terminated (not TSR).
                mov     ah, 09h
                mov     dx, OFFSET Bad_PasswordMsg
                int     21h
                mov     ah, 4Ch
                mov     al, 01h         ; ERRORLEVEL of 1.
                int     21h

Check_Done:
                ; Everything looks good. Return to caller.
                ret
Check_Password  ENDP

                COMMENT *
---------------------------------------------------------------------------
The following procedure installs the new keyboard interrupt service routine.
---------------------------------------------------------------------------
*

Hook_Keyboard   PROC    NEAR
                ; Get the address of the current keyboard ISR.
                mov     ah, 35h
                mov     al, 09h
                int     21h
                mov     OldKey_Off, bx
                mov     OldKey_Seg, es

                ; Set the keyboard interrupt vector to point at my ISR.
                mov     ah, 25h
                mov     al, 09h
                mov     dx, OFFSET Keyboard_ISR
                push    ds
                mov     bx, SEG Keyboard_ISR
                mov     ds, bx
                int     21h
                pop     ds
                ret
Hook_Keyboard   ENDP

                COMMENT *
---------------------------------------------------------------------------
The following procedure installs the new timer interrupt service routine.
---------------------------------------------------------------------------
*

Hook_Timer      PROC    NEAR
                ; Get the address of the current timer ISR.
                mov     ah, 35h
                mov     al, 08h
                int     21h
                mov     OldTim_Off, bx
                mov     OldTim_Seg, es

                ; Set the timer interrupt vector to point at my ISR.
                mov     ah, 25h
                mov     al, 08h
                mov     dx, OFFSET Timer_ISR
                push    ds
                mov     bx, SEG Timer_ISR
                mov     ds, bx
                int     21h
                pop     ds
                ret
Hook_Timer      ENDP

TCodeSegment    ENDS

;===========================================================================

StackSegment    SEGMENT STACK 'STACK'
                DB      2048 DUP (?)
StackSegment    ENDS

                END Start

