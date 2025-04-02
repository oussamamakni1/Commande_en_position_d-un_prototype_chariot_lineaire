 
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "mode.h"
#include "pwm.h"
#include "eqep.h"



void state_machine(struct status * slide_status)
{
	unsigned char exit = 1;
	
	while (exit)
	{
		switch (slide_status->mode)
		{
			case START:
				mode_start(slide_status);
				break;
			case RUN:
				mode_run(slide_status);
				break;
			case STOP:
				mode_stop(slide_status);
				exit = 0 ;
				break;
            case IDLE:
                mode_idle(slide_status);
                break;
			default:
				break;
		}
		usleep(500000);
	}
}

void mode_start(struct status * slide_status)
{
	printf("mode = START \n");
	set_pwm(EHRPWM1A, PWM_RUN, 1);
	set_gpio(GPIO0_31, DIRECTION, 0);
	set_pwm(EHRPWM1A, PWM_PERIOD, 5000);
	set_pwm(EHRPWM1A, PWM_DUTY, 0);
	set_pwm(EHRPWM1A, PWM_POLARITY, 0);
	

	set_eqep(EQEP1, POSITION, 0);
	set_eqep(EQEP1, ENABLED, 1);
	set_eqep(EQEP1, MODE, 0);
	slide_status->mode = 11;
    
	//slide_status->mode = RUN;


}
 #define DT 0.001
void mode_run(struct status * slide_status)
{
	printf("mode = RUN \n");
	
	int pos; 
    
	int kp = 14;
	int ki = 0.2;
	int  erreur_precedente = 0;
	int somme_erreur = 0;
    int x_t = 0;
	int erreur;

	struct  timeval t_ref, t_c;
	float dt_f =0; 
	 gettimeofday(&t_ref, NULL);
	while (slide_status->mode == RUN){
		do {
			gettimeofday(&t_c, NULL);
			dt_f = (t_c.tv_sec - t_ref.tv_sec)+ (t_c.tv_usec - t_ref.tv_usec)* 1e-6; 
		} while (dt_f < DT);

		t_ref = t_c;
		dt_f=0;

		x_t = get_eqep(EQEP1, POSITION);
		
		
		erreur = slide_status->des_pos - x_t;
		
		somme_erreur += erreur * dt_f;
		int commande = kp* erreur + ki*somme_erreur;
		erreur_precedente = erreur;
		//printf("la commande = %d",commande);
		
		if (commande > 0) {
			set_gpio(GPIO0_31, VALUE, 0);
		}
		else{
		set_gpio(GPIO0_31, VALUE, 1);}
		int commande_abs = abs(commande);
		if (commande_abs>5000){
		commande_abs = 5000;}
		/* if (commande >= 5000){
		commande = 5000;
		set_gpio(GPIO0_31, VALUE, 0);}
		else if (commande <= -5000){
			commande= -5000;
		set_gpio(GPIO0_31, VALUE, 1);} */
		
		set_pwm(EHRPWM1A, PWM_DUTY, commande_abs );
		//pos=get_eqep(EQEP1, POSITION);
	    //printf("la position actuelle = %d",pos);
		
	}
	
}

void mode_stop(struct status * slide_status)
{
	printf("mode = STOP \n");
    set_pwm(EHRPWM1A, PWM_RUN, 0);
	set_eqep(EQEP1, ENABLED, 0);
	pthread_mutex_destroy(&(slide_status->access));		
}

void mode_idle(struct status * slide_status)
{
	printf("mode = IDLE \n");
	set_pwm(EHRPWM1A, PWM_DUTY, 0);    
    set_pwm(EHRPWM1A, PWM_RUN, 0);      
    set_eqep(EQEP1, ENABLED, 0);
	slide_status->mode = 11;
	
}

void * ihm(void * param)
{
    struct status *slide_status = (struct status *) param;
    int cmd;
    float X_f, V_f;

    while (1)
    {
        printf("\n========= MENU =========\n");
        printf("0 : START\n");
        printf("1 : IDLE\n");
        printf("2 : STOP\n");
        printf("3 : GETX (Afficher position)\n");
        printf("4 : SETX (Définir position & vitesse)\n");
        printf("Choix : ");
        scanf("%d", &cmd);

        pthread_mutex_lock(&(slide_status->access)); 

        switch (cmd)
        {
            case 0:
                printf("\tSTART...\n");
                slide_status->mode = START;
                break;
            case 1:
                printf("\tIDLE...\n");
                slide_status->mode = IDLE;
                break;
            case 2:
                printf("\tSTOP...\n");
                slide_status->mode = STOP;
                pthread_mutex_unlock(&(slide_status->access)); 
                return NULL; 
            case 3:
                printf("\tPosition actuelle : %.2f m\n", slide_status->act_pos);
                break;
            case 4:
                printf("\tNouvelle position (m) : ");
                scanf("%f", &X_f);
                
                slide_status->des_pos = X_f;
                
                slide_status->mode = RUN;
                break;
            default:
                printf("\tCommande invalide !\n");
                break;
        }

        pthread_mutex_unlock(&(slide_status->access)); 
        usleep(500000); 
    }

    return NULL;
}