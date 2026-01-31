#include "modding.h"
#include "global.h"

s16 r_activation_flag = false;
s16 rupees_conversion = 0;
s16 rupees_100 = 0;
s16 rupees_10 = 0;
s16 rupees_1 = 0;
// unk120C0 = codeBufOffset
// unk120C2 = inputDigitIndex (aka cursor position)

// int 48 to 57 = '0' to '9'

RECOMP_HOOK("Message_ShouldAdvance") void Message_ShouldAdvance_Hook(PlayState* play) {
    MessageContext* msgCtx = &play->msgCtx;
    if (r_activation_flag == true) {

        // If the R button is pressed...
        if CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_R) {

            // Set rupees_conversion to be equal to currently held rupees for calculations
            rupees_conversion = gSaveContext.save.saveInfo.playerData.rupees;
            rupees_100 = 0;
            rupees_10 = 0;
            rupees_1 = 0;

            /*
             * Calculation process
             * With rupees_conversion being the amount of currently held rupees, we want to remove 1 from each decimal
             * slot, then add 1 to the corresponding rupees_xxx.
             * 
             * Example:
             * If we can remove 100 from rupees_conversion without falling below 0, remove 100 from rupees_conversion
             * and add 1 to rupees_100. This process is repeated until the count falls below 100. At this point, the
             * while loop will then check if it can remove 10 from rupees_conversion. It will eventually do the same
             * process with rupees_1 until rupees_conversion reaches 0.
             */
            while (rupees_conversion != 0) {
                if (rupees_conversion - 100 >= 0) {
                    rupees_conversion -= 100;
                    rupees_100 += 1;
                }
                else if (rupees_conversion - 10 >= 0) {
                    rupees_conversion -= 10;
                    rupees_10 += 1;
                }
                else if (rupees_conversion - 1 >= 0) {
                    rupees_conversion -= 1;
                    rupees_1 += 1;
                }
            }

            /*
             * The next three lines will use the values from the three rupees_xxx to tell the game how many rupees the
             * player wants to deposit. Displaying those values on screen are handled after.
             */
            msgCtx->decodedBuffer.schar[msgCtx->unk120C0] = rupees_100 + '0';
            msgCtx->decodedBuffer.schar[msgCtx->unk120C0 + 1] = rupees_10 + '0';
            msgCtx->decodedBuffer.schar[msgCtx->unk120C0 + 2] = rupees_1 + '0';

            /*
             * The rest is for displaying the desired deposit amount.
             *
             * Process of the for loop:
             * - Move the cursor to the right;
             * - If the cursor is already to the right-most slot, loop it around to the left-most slot instead;
             * - Load the character that corresponds to the decimal slot from the desired deposit amount;
             * - Repeat this process two more times for the other two decimals.
             */
            for (size_t i = 0; i < 3; i++) {
                msgCtx->unk120C2 += 1;
                if (msgCtx->unk120C2 > 2) {
                    msgCtx->unk120C2 = 0;
                }
            Font_LoadCharNES(play, msgCtx->decodedBuffer.schar[msgCtx->unk120C0 + msgCtx->unk120C2],
                             msgCtx->unk120C4 + (msgCtx->unk120C2 << 7));
            }

            // Play a sound effect to serve as an audio feedback as confirmation of button press.
            Audio_PlaySfx(NA_SE_SY_RUPY_COUNT);
        }
    }
}

// Message_ControlBankInput
// Allows pressing R to set the deposit amount equal to current amount held in wallet
RECOMP_HOOK("func_80148D64") void Activate_R_To_Deposit() {
    r_activation_flag = true;
}

RECOMP_HOOK("Message_DrawMain") void Deactivate_R_To_Deposit() {
    r_activation_flag = false;
}