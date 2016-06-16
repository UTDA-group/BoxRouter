open(FH_IN,"$ARGV[0]")||die "Cannot open file $ARGV[0]";
open(FH_OU,">$ARGV[1]")||die "Cannot open file $ARGV[1]";

$column_on = 0;
@columns = ();
@mps_txt = ();
while(<FH_IN>)
{
    #f($line=~/^WPIN\s+M(\d+)\s+(-?\d+)\s+(-?\d+)\s+(\d+)\s+(\d+)\s*(.*)\s*/)
    
    if($_=~/^COLUMNS/)
    {
        $column_on = 1;
        print FH_OU $_;
        next;
     }
    
    if($column_on==1)
    {
        if($_=~/^\s+/)
        {
            push @columns,$_;
        }
        else
        {
            $column_on = 0;
            foreach(sort @columns)
            {
                print FH_OU $_;
            }
            print FH_OU $_;
        }         
    }
    else
    {
        print FH_OU $_;        
    }
}
close FH_IN;
close FH_OU;

