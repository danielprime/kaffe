/* Length.java -- Converts CSS length values
   Copyright (C) 2006 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


package gnu.javax.swing.text.html.css;

/**
 * Converts CSS length values to usable length values.
 *
 * @author Roman Kennke (kennke@aicas.com)
 */
public class Length
{

  /**
   * The original value.
   */
  private String value;

  /**
   * The converted value.
   */
  private float floatValue;

  /**
   * Indicates when the value is a percentage value.
   */
  private boolean isPercentage;

  /**
   * Creates a new length converter instance.
   *
   * @param val the CSS value
   */
  public Length(String val)
  {
    value = val;
    int i = value.indexOf("px");
    int percent = value.indexOf("%");
    floatValue = 0.0F;
    if (i != -1)
      {
        String sub = value.substring(0, i);
        floatValue = Float.parseFloat(sub);
      }
    else if (percent != -1)
      {
        isPercentage = true;
        String sub = value.substring(0, percent);
        floatValue = Float.parseFloat(sub) / 100;
      }
    else
      {
        // TODO: Implement other length options.
        floatValue = Float.parseFloat(value);
      }
  }

  /**
   * Returns the value converted to pixels.
   *
   * @return the value converted to pixels
   */
  public float getValue()
  {
    return floatValue;
  }

  /**
   * Returns the absolute span for the case when this length value is
   * a relative value.
   *
   * @param span the target span
   *
   * @return the absolute span
   */
  public float getValue(float span)
  {
    return span * floatValue;
  }

  /**
   * Returns <code>true</code> when the length value is a percentage
   * value, <code>false</code> otherwise.
   *
   * @return <code>true</code> when the length value is a percentage
   *         value, <code>false</code> otherwise
   */
  public boolean isPercentage()
  {
    return isPercentage;
  }
}
