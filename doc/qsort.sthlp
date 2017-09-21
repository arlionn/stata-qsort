{smcl}
{* *! version 0.1.0  30Jun2017}{...}
{viewerdialog qsort "dialog sort, message(-qsort-)"}{...}
{vieweralsosee "[D] qsort" "mansection D qsort"}{...}
{vieweralsosee "" "--"}{...}
{vieweralsosee "[D] sort" "help sort"}{...}
{viewerjumpto "Syntax" "qsort##syntax"}{...}
{viewerjumpto "Menu" "qsort##menu"}{...}
{viewerjumpto "Description" "qsort##description"}{...}
{viewerjumpto "Options" "qsort##options"}{...}
{viewerjumpto "Examples" "qsort##examples"}{...}
{title:Title}

{p2colset 5 18 20 2}{...}
{p2col :{manlink D qsort} {hline 2}}Implementation of {opt sort} and {opt gsort} using C-plugins{p_end}
{p2colreset}{...}


{marker syntax}{...}
{title:Syntax}

{p 8 14 2}
{cmd:qsort}
[{cmd:+}|{cmd:-}]
{varname}
[[{cmd:+}|{cmd:-}]
{varname} {it:...}]


{marker menu}{...}
{title:Menu}

{phang}
{bf:Data > Sort}


{marker description}{...}
{title:Description}

{pstd}
{opt qsort} uses C-plugins to implement a faster version of {opt gsort}.
HOWEVER, this comes at a very large memory penalty. This is mainly a
proof-of-concept package written because the internals will be used
elsewhere. Like {opt gsort}, it can arrange observations to be in ascending or
descending order.

{pstd}
Each {varname} can be numeric or a string.

{pstd}
The observations are placed in ascending order of {it:varname} if {opt +}
or nothing is typed in front of the name and are placed in descending order if
{opt -} is typed.


{marker options}{...}
{title:Options}


{marker examples}{...}
{title:Examples}

    {hline}
    Setup
{phang2}{cmd:. sysuse auto}

{pstd}Place observations in ascending order of {cmd:price}{p_end}
{phang2}{cmd:. qsort price}

{pstd}Same as above command{p_end}
{phang2}{cmd:. qsort +price} 

{pstd}List the 10 lowest-priced cars in the data{p_end}
{phang2}{cmd:. list make price in 1/10}

{pstd}Place observations in descending order of {cmd:price}{p_end}
{phang2}{cmd:. qsort -price}

{pstd}List the 10 highest-priced cars in the data{p_end}
{phang2}{cmd:. list make price in 1/10}

{pstd}Place observations in alphabetical order of {cmd:make}{p_end}
{phang2}{cmd:. qsort make}

{pstd}List {cmd:make} in alphabetical order{p_end}
{phang2}{cmd:. list make}

{pstd}Place observations in reverse alphabetical order of {cmd:make}{p_end}
{phang2}{cmd:. qsort -make}

{pstd}List {cmd:make} in reverse alphabetical order{p_end}
{phang2}{cmd:. list make}

    {hline}
    Setup
{phang2}{cmd:. webuse bp3}

{pstd}Place observations in ascending order of {cmd:time} within ascending
order of {cmd:id}{p_end}
{phang2}{cmd:. qsort id time}

{pstd}List each patient's blood pressures in the order measurements were
taken{p_end}
{phang2}{cmd:. list id time bp}

{pstd}Place observations in descending order of {cmd:time} within ascending
order of {cmd:id}{p_end}
{phang2}{cmd:. qsort id -time}

{pstd}List each patient's blood pressures in reverse-time order{p_end}
{phang2}{cmd:. list id time bp}{p_end}
    {hline}
