      subroutine perma(compo)
      include "calcium.hf"

      dimension tparoi(100),tp(100)
      character*(INSTANCE_LEN) nom_instance
      integer info
      integer compo

c
c  boucle temporelle infinie     
c
      do while( .TRUE. )    
c
c   lecture de la temperature de paroi a t
c
        CALL cplRE(compo,CP_SEQUENTIEL,t, t, ii, 'tparoi', 100,
     &             nval, tparoi, info)

        IF( info.NE. CPOK  ) GO TO 9000
c
c  Ici on realise l'interpolation
c
        do i=1,nval
          tp(i)=tparoi(i)
        enddo
c
c  ecriture de la temperature de paroi interpolee a t
c
        CALL cpeRE(compo,CP_ITERATION, t, ii, 'tpar', nval,
     &             tp , info)

        IF( info.NE.CPOK ) GO TO 9000
c       if(ii .GE. 24)GO TO 9000

      enddo

9000  continue
            CALL cpfin(compo,CP_ARRET, info)
      end
