      subroutine transit(compo)

      include "calcium.hf"

      dimension tparoi(100),tp(100)
      character*(INSTANCE_LEN)  nom_instance
      integer  info
      integer compo
c
c  boucle temporelle infinie     
c
      do while( .TRUE. )    
c
c   lecture de la temperature de paroi a t
c
      t=0.
        CALL cplRE(compo,CP_SEQUENTIEL,t, t, ii, 'tparoit', 100,
     &             nval, tparoi, info)

        IF( info.NE.CPOK ) GO TO 9000
c
c  Ici on realise l'interpolation
c
        do i=1,nval
          tp(i)=tparoi(i)
        enddo
c
c  ecriture de la temperature de paroi interpolee a t
c
        write(6,*)'INTERP:temps=',t
        call flush(6)

        CALL cpere(compo,CP_TEMPS, t, ii, 'tpart', nval,
     &             tp , info)

        IF( info.NE.CPOK ) GO TO 9000
c       if(t .gt. 100.)GO TO 9000

      enddo

9000  continue

            CALL cpfin(compo,CP_ARRET, info)

      end
